#!/bin/bash
# run from the repo root:  bash tests/person_c/run_person_c_tests.sh
set -e
SRCS=$(ls src/*.cpp | grep -v '/main.cpp')
g++ -std=c++17 -Wall -Wextra -Isrc $SRCS tests/person_c/person_c_test.cpp -o person_c_test
t() {  # t <input-file> <mode> <atm> <account> [prior] [topup]
  f=$1; shift
  echo; echo "##### $f   (person_c_test $*)"
  ./person_c_test "$@" < tests/person_c/$f.in | grep -E "Error|completed|history|Account \[|ATM \[|before|after|Dispens|KRW [0-9]+ x|Fee charged|Deposit fee" | sed 's/^/   /'
}
t tc01_cash_deposit_primary             deposit  1 1
t tc02_cash_deposit_nonprimary_with_fee deposit  2 1
t tc03_check_deposit_ok                 deposit  1 1
t tc04_check_too_small                  deposit  1 1
t tc05_wrong_fee                        deposit  2 1
t tc06_deposit_zero_cash                deposit  1 1
t tc07_withdraw_primary_ok              withdraw 1 1
t tc08_withdraw_not_enough_funds        withdraw 1 1
t tc09_withdraw_nonprimary_fee3000      withdraw 2 1
t tc10_withdraw_not_multiple_of_1000    withdraw 1 1 0 100000
t tc11_fourth_withdrawal_refused        withdraw 1 1 3 100000
t tc12_withdraw_17000_fewest_bills      withdraw 1 1 0 100000
t tc13_withdraw_over_500000             withdraw 1 1 0 1000000
t tc14_withdraw_atm_has_not_enough_cash withdraw 1 1 0 1000000
