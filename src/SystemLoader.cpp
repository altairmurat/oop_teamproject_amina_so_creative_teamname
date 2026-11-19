// Owner: Seunggyun (Person A)
#include "SystemLoader.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>

const std::string SystemLoader::DEFAULT_ADMIN_CARD = "0000";

namespace {

struct Line {
    int number;       // 1-based line number in the file, for error messages
    std::string text; // trimmed
};

std::string trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

// Lowercase and drop '-', '_' and spaces: "Single-Bank" -> "singlebank".
std::string simplify(const std::string& s) {
    std::string out;
    for (char c : s)
        if (c != '-' && c != '_' && !std::isspace(static_cast<unsigned char>(c)))
            out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

bool allDigits(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(),
                                     [](unsigned char c) { return std::isdigit(c); });
}

[[noreturn]] void fail(const Line& l, const std::string& msg) {
    throw std::runtime_error("Line " + std::to_string(l.number) + ": " + msg +
                             " (got \"" + l.text + "\")");
}

// Non-negative integer; commas allowed ("4,000"). Rejects anything else.
long long parseAmount(const Line& l, const std::string& field) {
    std::string s;
    for (char c : l.text) if (c != ',') s += c;
    if (!allDigits(s) || s.size() > 15) fail(l, field + " must be a non-negative integer");
    return std::stoll(s);
}

class Reader {
public:
    explicit Reader(std::vector<Line> lines) : lines(std::move(lines)) {}
    bool done() const { return pos >= lines.size(); }
    const Line& next(const std::string& what) {
        if (done()) throw std::runtime_error("Unexpected end of file: expected " + what);
        return lines[pos++];
    }
private:
    std::vector<Line> lines;
    size_t pos = 0;
};

} // namespace

SystemData SystemLoader::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open initial condition file: " + path);

    std::vector<Line> lines;
    std::string raw;
    for (int n = 1; std::getline(in, raw); ++n) {
        std::string t = trim(raw); // also removes '\r' from Windows files
        if (t.empty() || t[0] == '#') continue;
        lines.push_back({n, t});
    }
    Reader r(std::move(lines));
    SystemData data;

    // ---- header ----
    const Line& head = r.next("header \"B A N\"");
    std::istringstream hs(head.text);
    long long B, A, N;
    std::string extra;
    if (!(hs >> B >> A >> N) || (hs >> extra))
        fail(head, "header must be three integers \"B A N\"");
    if (B < 1 || A < 0 || N < 1)
        fail(head, "need at least 1 bank and 1 ATM, and a non-negative account count");

    // ---- banks ----
    for (long long i = 0; i < B; ++i) {
        const Line& l = r.next("bank name");
        if (Bank::findBank(l.text)) fail(l, "duplicate bank name");
        data.banks.push_back(std::make_unique<Bank>(l.text));
    }

    // ---- accounts ----
    for (long long i = 0; i < A; ++i) {
        const Line& bankL = r.next("account bank name");
        Bank* bank = Bank::findBank(bankL.text);
        if (!bank) fail(bankL, "unknown bank");

        const Line& userL = r.next("user name");
        const Line& numL = r.next("account number");
        std::string digits = Account::normalize(numL.text);
        if (!allDigits(digits) || digits.size() != 12)
            fail(numL, "account number must have exactly 12 digits");
        if (Bank::findAccount(numL.text)) fail(numL, "duplicate account number");

        const Line& fundsL = r.next("available funds");
        long long funds = parseAmount(fundsL, "funds");

        const Line& cardL = r.next("card number");
        const Line& pwL = r.next("password");

        bank->openAccount(userL.text, numL.text, funds, cardL.text, pwL.text);
    }

    // ---- ATMs ----
    std::set<std::string> serials;
    for (long long i = 0; i < N; ++i) {
        ATMConfig cfg;

        const Line& bankL = r.next("ATM primary bank");
        if (!Bank::findBank(bankL.text)) fail(bankL, "unknown bank");
        cfg.primaryBank = bankL.text;

        const Line& snL = r.next("ATM serial number");
        if (!allDigits(snL.text) || snL.text.size() != 6)
            fail(snL, "serial number must be exactly 6 digits");
        if (!serials.insert(snL.text).second) fail(snL, "duplicate ATM serial number");
        cfg.serialNumber = snL.text;

        const Line& typeL = r.next("ATM type");
        std::string t = simplify(typeL.text);
        if (t == "single" || t == "singlebank") cfg.multiBank = false;
        else if (t == "multi" || t == "multibank") cfg.multiBank = true;
        else fail(typeL, "type must be Single or Multi");

        const Line& langL = r.next("ATM language");
        std::string lg = simplify(langL.text);
        if (lg == "unilingual" || lg == "uni") cfg.bilingual = false;
        else if (lg == "bilingual" || lg == "bi") cfg.bilingual = true;
        else fail(langL, "language must be Unilingual or Bilingual");

        const Line& cashL = r.next("ATM cash counts");
        std::string c = cashL.text;
        std::replace(c.begin(), c.end(), ',', ' ');
        std::replace(c.begin(), c.end(), '/', ' ');
        std::istringstream cs(c);
        for (int& count : cfg.bills) {
            std::string tok;
            if (!(cs >> tok) || !allDigits(tok) || tok.size() > 9)
                fail(cashL, "cash must be 4 non-negative counts: 50k 10k 5k 1k");
            count = std::stoi(tok);
        }
        if (cs >> extra) fail(cashL, "cash must be exactly 4 counts");

        data.atms.push_back(cfg);
    }

    // ---- optional admin cards ----
    bool anyAdmin = false;
    while (!r.done()) {
        const Line& l = r.next("");
        std::istringstream as(l.text);
        std::string word, card;
        if ((as >> word >> card) && simplify(word) == "admin" && !(as >> extra)) {
            if (Bank::findAccountByCard(card)) fail(l, "admin card number is already a user card");
            Bank::registerAdminCard(card);
            anyAdmin = true;
        } else {
            std::cerr << "[warning] line " << l.number << " ignored: \"" << l.text << "\"\n";
        }
    }
    if (!anyAdmin && !Bank::findAccountByCard(DEFAULT_ADMIN_CARD))
        Bank::registerAdminCard(DEFAULT_ADMIN_CARD);

    return data;
}
