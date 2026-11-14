#include "Messages.h"
#include <map>

using namespace std;

// оба словаря должны иметь одинаковые ключи, если что-то добавляете то в оба пж
static map<string, string> createEnglishTable() {
    map<string, string> t;

    t["select_language"] = "Select language (1. English, 2. Korean): ";
    t["insert_card"] = "Please insert your card (enter card number): ";
    t["enter_password"] = "Enter password: ";
    t["wrong_password"] = "Wrong password. Please try again.";
    t["auth_failed"] = "Too many wrong password attempts. Session ended.";
    t["invalid_card"] = "This card cannot be used on this ATM.";
    t["session_end"] = "Session ended. Please take your card.";
    t["session_summary"] = "=== Session Summary ===";

    t["main_menu"] = "\n1. Deposit\n2. Withdrawal\n3. Transfer\n4. Cancel\nSelect: ";
    t["choose_transfer_type"] = "\n1. Cash Transfer\n2. Account Transfer\nSelect: ";

    t["enter_dest_account"] = "Enter destination account number: ";
    t["enter_amount"] = "Enter amount: ";
    t["insert_cash_bills"] = "Insert bills for each denomination:";
    t["invalid_input"] = "Invalid input.";
    t["account_not_found"] = "Account not found.";
    t["same_account"] = "Cannot transfer to the same account.";

    t["insufficient_funds"] = "Insufficient funds in your account.";
    t["insufficient_cash"] = "ATM does not have enough cash.";
    t["limit_exceeded"] = "Transaction limit exceeded.";

    t["fee_notice"] = "Fee (KRW): ";
    t["transfer_success"] = "Transfer completed successfully.";
    t["cash_transfer_label"] = "Cash Transfer";
    t["account_transfer_label"] = "Account Transfer";

    t["admin_menu"] = "\n[Admin Menu]\n1. View Transaction History\n2. Exit\nSelect: ";
    t["history_header"] = "=== Transaction History ===";
    t["history_empty"] = "No transactions recorded yet.";
    t["history_saved"] = "Transaction history saved to file: ";

    return t;
}

static map<string, string> createKoreanTable() {
    map<string, string> t;

    t["select_language"] = "언어를 선택하세요 (1. 영어, 2. 한국어): ";
    t["insert_card"] = "카드를 넣어주세요 (카드 번호 입력): ";
    t["enter_password"] = "비밀번호를 입력하세요: ";
    t["wrong_password"] = "비밀번호가 틀렸습니다. 다시 시도해주세요.";
    t["auth_failed"] = "비밀번호 오류 횟수를 초과했습니다. 세션을 종료합니다.";
    t["invalid_card"] = "이 ATM에서는 사용할 수 없는 카드입니다.";
    t["session_end"] = "거래가 종료되었습니다. 카드를 가져가세요.";
    t["session_summary"] = "=== 거래 요약 ===";

    t["main_menu"] = "\n1. 입금\n2. 출금\n3. 이체\n4. 종료\n선택: ";
    t["choose_transfer_type"] = "\n1. 현금 이체\n2. 계좌 이체\n선택: ";

    t["enter_dest_account"] = "받는 분의 계좌번호를 입력하세요: ";
    t["enter_amount"] = "금액을 입력하세요: ";
    t["insert_cash_bills"] = "권종별 지폐 매수를 입력하세요:";
    t["invalid_input"] = "잘못된 입력입니다.";
    t["account_not_found"] = "계좌를 찾을 수 없습니다.";
    t["same_account"] = "같은 계좌로는 이체할 수 없습니다.";

    t["insufficient_funds"] = "계좌 잔액이 부족합니다.";
    t["insufficient_cash"] = "ATM에 현금이 부족합니다.";
    t["limit_exceeded"] = "거래 한도를 초과했습니다.";

    t["fee_notice"] = "수수료 (원): ";
    t["transfer_success"] = "이체가 완료되었습니다.";
    t["cash_transfer_label"] = "현금 이체";
    t["account_transfer_label"] = "계좌 이체";

    t["admin_menu"] = "\n[관리자 메뉴]\n1. 거래 내역 조회\n2. 종료\n선택: ";
    t["history_header"] = "=== 전체 거래 내역 ===";
    t["history_empty"] = "기록된 거래가 없습니다.";
    t["history_saved"] = "거래 내역이 파일로 저장되었습니다: ";

    return t;
}

string Messages::get(const string& key, Language lang) {
    static const map<string, string> en = createEnglishTable();
    static const map<string, string> kr = createKoreanTable();

    const map<string, string>& table = (lang == Language::Korean) ? kr : en;
    auto it = table.find(key);
    if (it != table.end()) {
        return it->second;
    }
    // key is returned if not founded so program is not crashed
    return key;
}
