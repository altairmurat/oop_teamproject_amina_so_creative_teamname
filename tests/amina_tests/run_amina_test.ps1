# run from the repo root
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$src = Get-ChildItem src\*.cpp | Where-Object { $_.Name -ne "main.cpp" } | ForEach-Object { $_.FullName }
g++ -std=c++17 -Wall -Wextra -Isrc $src tests\amina_tests\amina_test.cpp -o amina_test.exe
if ($LASTEXITCODE -ne 0) { Write-Host "BUILD FAILED"; exit 1 }

function T($name, $testArgs) {
    Write-Host ""
    Write-Host "##### $name   ($($testArgs -join ' '))"
    Get-Content "tests\amina_tests\$name.in" | & .\amina_test.exe @testArgs |
        Select-String -Pattern "Error|completed|history|Account \[|ATM \[|before|after|Dispens|KRW [0-9]+ x|Fee charged|Deposit fee"
}

T tc01_cash_deposit_primary             @("deposit","1","1")
T tc02_cash_deposit_nonprimary_with_fee @("deposit","2","1")
T tc03_check_deposit_ok                 @("deposit","1","1")
T tc04_check_too_small                  @("deposit","1","1")
T tc05_wrong_fee                        @("deposit","2","1")
T tc06_deposit_zero_cash                @("deposit","1","1")
T tc07_withdraw_primary_ok              @("withdraw","1","1")
T tc08_withdraw_not_enough_funds        @("withdraw","1","1")
T tc09_withdraw_nonprimary_fee3000      @("withdraw","2","1")
T tc10_withdraw_not_multiple_of_1000    @("withdraw","1","1","0","100000")
T tc11_fourth_withdrawal_refused        @("withdraw","1","1","3","100000")
T tc12_withdraw_17000_fewest_bills      @("withdraw","1","1","0","100000")
T tc13_withdraw_over_500000             @("withdraw","1","1","0","1000000")
T tc14_withdraw_atm_has_not_enough_cash @("withdraw","1","1","0","1000000")