#include "Messages.h"
#include <map>

using namespace std;

// оба словаря должны иметь одинаковые ключи, если что-то добавляете то в оба пж
string Messages::get(const string& key, Language lang) {
    static map<string, string> en;
    static map<string, string> kr;
    static bool loaded = false;

    if (!loaded) {
        // english
        en["select_language"] = "Select language (1. English, 2. Korean): ";
        en["insert_card"] = "Insert card (enter card number): ";
        en["enter_password"] = "Enter password: ";
        en["wrong_password"] = "Wrong password. Please try again.";
        en["auth_failed"] = "Too many wrong password attempts. Session ended.";
        en["invalid_card"] = "This card cannot be used on this ATM.";
        en["session_end"] = "Session ended. Please take your card.";
        en["session_summary"] = "=== Session Summary ===";

        en["main_menu"] = "\n1. Deposit\n2. Withdrawal\n3. Transfer\n4. Cancel\nSelect: ";
        en["choose_transfer_type"] = "\n1. Cash Transfer\n2. Account Transfer\nSelect: ";

        en["enter_dest_account"] = "Enter destination account number: ";
        en["enter_amount"] = "Enter transfer amount (KRW): ";
        en["insert_cash_bills"] = "Insert cash bills (includes KRW 1,000 fee):";
        en["invalid_input"] = "Invalid input.";
        en["account_not_found"] = "Account not found.";
        en["bank_not_found"] = "Destination bank not found.";
        en["same_account"] = "Cannot transfer to the same account.";
        en["cash_transfer_too_small"] = "Inserted cash must be more than 1,000 KRW fee.";

        en["insufficient_funds"] = "Insufficient funds in your account.";
        en["insufficient_cash"] = "ATM does not have enough cash.";
        en["limit_exceeded"] = "Transaction limit exceeded.";

        en["fee_notice"] = "Fee (KRW): ";
        en["transfer_success"] = "Transfer completed.";
        en["cash_transfer_label"] = "Cash Transfer";
        en["account_transfer_label"] = "Account Transfer";

        en["admin_menu"] = "\n[Admin Menu]\n1. View Transaction History\n2. Exit\nSelect: ";
        en["history_header"] = "=== Transaction History ===";
        en["history_empty"] = "No transactions recorded yet.";
        en["history_saved"] = "Transaction history saved to file: ";

        // ключи от Амины (deposit / withdrawal / bills), оставил названия как у нее в коде
        en["prompt.bills_of"] = "Number of bills for KRW ";
        en["err.too_many_bills"] = "Error: too many bills (maximum 50 per transaction).";
        en["deposit.choose_type"] = "Deposit type - 1) Cash  2) Check: ";
        en["deposit.enter_check"] = "Enter the check amount (KRW 100,000 or more): ";
        en["deposit.fee_label"] = "Deposit fee required - insert it in cash (KRW): ";
        en["err.check_too_small"] = "Error: invalid check (minimum KRW 100,000).";
        en["err.no_cash"] = "Error: no cash was inserted.";
        en["err.fee_mismatch"] = "Error: the inserted fee does not match the required fee.";
        en["deposit.success"] = "Deposit completed.";
        en["withdraw.enter_amount"] = "Enter the withdrawal amount (KRW): ";
        en["err.withdraw_count"] = "Error: maximum number of withdrawals per session (3) reached.";
        en["err.withdraw_max"] = "Error: maximum withdrawal per transaction is KRW 500,000.";
        en["err.amount_unit"] = "Error: amount must be a multiple of KRW 1,000.";
        en["err.no_funds"] = "Error: insufficient funds in the account (amount + fee).";
        en["err.no_cash_atm"] = "Error: insufficient cash in the ATM.";
        en["err.too_many_dispense"] = "Error: dispensing this amount would exceed 50 bills.";
        en["withdraw.dispensing"] = "Dispensing:";
        en["withdraw.fee_label"] = "Fee charged to the account (KRW): ";
        en["withdraw.success"] = "Withdrawal completed.";


        // korean
        kr["select_language"] = "언어를 선택하세요 (1. 영어, 2. 한국어): ";
        kr["insert_card"] = "카드를 넣어주세요 (카드 번호 입력): ";
        kr["enter_password"] = "비밀번호를 입력하세요: ";
        kr["wrong_password"] = "비밀번호가 틀렸습니다. 다시 시도해주세요.";
        kr["auth_failed"] = "비밀번호 오류 횟수를 초과했습니다. 세션을 종료합니다.";
        kr["invalid_card"] = "이 ATM에서는 사용할 수 없는 카드입니다.";
        kr["session_end"] = "거래가 종료되었습니다. 카드를 가져가세요.";
        kr["session_summary"] = "=== 거래 요약 ===";

        kr["main_menu"] = "\n1. 입금\n2. 출금\n3. 이체\n4. 종료\n선택: ";
        kr["choose_transfer_type"] = "\n1. 현금 이체\n2. 계좌 이체\n선택: ";

        kr["enter_dest_account"] = "받는 분 계좌번호를 입력하세요: ";
        kr["enter_amount"] = "이체할 금액을 입력하세요 (원): ";
        kr["insert_cash_bills"] = "권종별 지폐 매수를 입력하세요 (수수료 1,000원 포함):";
        kr["invalid_input"] = "잘못된 입력입니다.";
        kr["account_not_found"] = "계좌를 찾을 수 없습니다.";
        kr["bank_not_found"] = "받는 분의 은행을 찾을 수 없습니다.";
        kr["same_account"] = "같은 계좌로는 이체할 수 없습니다.";
        kr["cash_transfer_too_small"] = "투입 금액은 수수료 1,000원보다 커야 합니다.";

        kr["insufficient_funds"] = "계좌 잔액이 부족합니다.";
        kr["insufficient_cash"] = "ATM에 현금이 부족합니다.";
        kr["limit_exceeded"] = "거래 한도를 초과했습니다.";

        kr["fee_notice"] = "수수료 (원): ";
        kr["transfer_success"] = "이체가 완료되었습니다.";
        kr["cash_transfer_label"] = "현금 이체";
        kr["account_transfer_label"] = "계좌 이체";

        kr["admin_menu"] = "\n[관리자 메뉴]\n1. 거래 내역 조회\n2. 종료\n선택: ";
        kr["history_header"] = "=== 전체 거래 내역 ===";
        kr["history_empty"] = "기록된 거래가 없습니다.";
        kr["history_saved"] = "거래 내역이 파일로 저장되었습니다: ";

        // korean translations for Amina's keys (deposit and withdraw)
        kr["prompt.bills_of"] = "지폐 매수 입력 (KRW) ";
        kr["err.too_many_bills"] = "오류: 지폐 매수가 너무 많습니다 (최대 50장).";
        kr["deposit.choose_type"] = "입금 유형 - 1) 현금  2) 수표: ";
        kr["deposit.enter_check"] = "수표 금액을 입력하세요 (100,000원 이상): ";
        kr["deposit.fee_label"] = "입금 수수료 - 현금으로 투입하세요 (원): ";
        kr["err.check_too_small"] = "오류: 유효하지 않은 수표입니다 (최소 100,000원).";
        kr["err.no_cash"] = "오류: 투입된 현금이 없습니다.";
        kr["err.fee_mismatch"] = "오류: 투입한 수수료가 맞지 않습니다.";
        kr["deposit.success"] = "입금이 완료되었습니다.";
        kr["withdraw.enter_amount"] = "출금할 금액을 입력하세요 (원): ";
        kr["err.withdraw_count"] = "오류: 세션당 최대 출금 횟수(3회)를 초과했습니다.";
        kr["err.withdraw_max"] = "오류: 1회 최대 출금 한도는 500,000원입니다.";
        kr["err.amount_unit"] = "오류: 금액은 1,000원 단위여야 합니다.";
        kr["err.no_funds"] = "오류: 계좌 잔액이 부족합니다 (금액 + 수수료).";
        kr["err.no_cash_atm"] = "오류: ATM에 현금이 부족합니다.";
        kr["err.too_many_dispense"] = "오류: 출금 지폐가 50장을 초과합니다.";
        kr["withdraw.dispensing"] = "출금 지폐:";
        kr["withdraw.fee_label"] = "계좌에서 차감된 수수료 (원): ";
        kr["withdraw.success"] = "출금이 완료되었습니다.";

        loaded = true;
    }

    if(lang == Language::Korean) {
        if (kr.count(key)) return kr[key];
    } else {
        if (en.count(key)) return en[key];
    }

    // key is returned if not founded so program is not crashed
    return key;
}
