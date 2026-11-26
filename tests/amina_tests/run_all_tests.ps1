<#
.SYNOPSIS
    CSE201 ATM term project: runs every test case against the real program and saves
    console transcripts for the final report.  Owner: Amina (Person C, test plan & QA).

.DESCRIPTION
    1. Builds atm.exe plus the unit-test programs (g++/MinGW or Visual Studio cl).
    2. Runs the unit tests (bank side, Person D, Amina's deposit/withdrawal harness).
    3. Runs every end-to-end scenario. Each scenario restarts atm.exe, so every test
       starts from the same initial conditions, exactly as in the live demo.
    4. Checks the output against the expected result. Each test is PASS or FAIL.
    5. Writes everything into test_results\ :
         transcripts\<id>.txt  console output with the typed inputs shown after each
                               prompt (open it, or use -Show <id>, then screenshot it)
         results.csv           one row per test (id, REQ, result, why it is reasonable)
         checklist.md          REQ-by-REQ O/X draft for the requirement check sheet

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File tests\amina_tests\run_all_tests.ps1
.EXAMPLE
    .\tests\amina_tests\run_all_tests.ps1 -Build              # force a clean rebuild
.EXAMPLE
    .\tests\amina_tests\run_all_tests.ps1 -Only W01,T01       # run just some tests
.EXAMPLE
    .\tests\amina_tests\run_all_tests.ps1 -Show W01           # print a transcript for a screenshot
.EXAMPLE
    .\tests\amina_tests\run_all_tests.ps1 -NoEcho             # faster, transcripts without typed inputs
#>
[CmdletBinding()]
param(
    [switch]$Build,
    [switch]$NoEcho,
    [string[]]$Only,
    [string]$Show,
    [string]$OutDir = 'test_results'
)

$ErrorActionPreference = 'Continue'   # 'Stop' would abort on compiler warnings in Windows PowerShell 5.1

# ---------------------------------------------------------------- setup
$Root = $PSScriptRoot
while ($Root -and -not (Test-Path (Join-Path $Root 'src\main.cpp'))) { $Root = Split-Path $Root -Parent }
if (-not $Root) {
    if (Test-Path (Join-Path (Get-Location).Path 'src\main.cpp')) { $Root = (Get-Location).Path }
    else { Write-Host 'Cannot find the project folder (src\main.cpp). Put this script in tests\amina_tests\ of the repo.' -ForegroundColor Red; exit 2 }
}
Set-Location $Root

try { & chcp.com 65001 > $null } catch {}
$Utf8    = New-Object System.Text.UTF8Encoding $false
$Utf8Bom = New-Object System.Text.UTF8Encoding $true
try { [Console]::OutputEncoding = $Utf8 } catch {}

if (-not [IO.Path]::IsPathRooted($OutDir)) { $OutDir = Join-Path $Root $OutDir }
$TranscriptDir = Join-Path $OutDir 'transcripts'

if ($Show) {
    $f = Get-ChildItem $TranscriptDir -Filter "$Show*.txt" -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $f) { Write-Host "No transcript matching '$Show' in $TranscriptDir. Run the tests first." -ForegroundColor Red; exit 2 }
    Clear-Host
    [Console]::Out.Write([IO.File]::ReadAllText($f.FullName, $Utf8))
    exit 0
}

$BuildDir = Join-Path $Root 'build_ps'
$TmpDir   = Join-Path $OutDir 'tmp'
$RunDir   = Join-Path $OutDir 'run'
foreach ($d in @($BuildDir, $OutDir, $TranscriptDir, $TmpDir, $RunDir)) { New-Item -ItemType Directory -Force -Path $d | Out-Null }

# ---------------------------------------------------------------- test data files
$ExtData = @'
2 4 3
Kakao
IM
Kakao
Chris
100-100-111111
4000
1111-1111-1111
1234
IM
Andor
200-200-222222
6000
2222-2222-2222
4321
Kakao
Chris
100-100-333333
500000
3333-3333-3333
1111
IM
Chris
200-200-444444
100000
4444-4444-4444
2222
Kakao
000111
Single
Unilingual
2 3 1 3
IM
000222
Multi
Bilingual
0 0 0 60
Kakao
000333
Multi
Unilingual
10 10 10 10
'@
$BrokenData = @'
2 2 2
Kakao
IM
Kakao
Chris
100-100-111111
4000
1111-1111-1111
1234
IM
Andor
200-200-222222
6000
2222-2222-2222
4321
Kakao
00011
Single
Unilingual
2 3 1 3
IM
000222
Multi
Bilingual
0 0 0 60
'@
$DataFiles = @{
    'main'   = (Join-Path $Root 'data\initial_conditions.txt')
    'ext'    = (Join-Path $OutDir 'conditions_extended.txt')
    'broken' = (Join-Path $OutDir 'conditions_broken.txt')
}
[IO.File]::WriteAllText($DataFiles['ext'],    ($ExtData -replace "`r`n", "`n") + "`n", $Utf8)
[IO.File]::WriteAllText($DataFiles['broken'], ($BrokenData -replace "`r`n", "`n") + "`n", $Utf8)

# ---------------------------------------------------------------- build
$SrcDir  = Join-Path $Root 'src'
$AllSrc  = @(Get-ChildItem $SrcDir -Filter *.cpp | ForEach-Object { $_.FullName })
$NoMain  = @($AllSrc | Where-Object { (Split-Path $_ -Leaf) -ne 'main.cpp' })
$BankSrc = @('Card', 'Account', 'Bank', 'SystemLoader') | ForEach-Object { Join-Path $SrcDir "$_.cpp" }

$Exe = @{
    'atm'    = (Join-Path $BuildDir 'atm.exe')
    'bank'   = (Join-Path $BuildDir 'bank_side_test.exe')
    'persd'  = (Join-Path $BuildDir 'person_d_test.exe')
    'amina'  = (Join-Path $BuildDir 'amina_test.exe')
}
$Sources = @{
    'atm'   = $AllSrc
    'bank'  = @($BankSrc + (Join-Path $Root 'tests\bank_side_test.cpp'))
    'persd' = @($NoMain + (Join-Path $Root 'tests\person_d_test.cpp'))
    'amina' = @($NoMain + (Join-Path $Root 'tests\amina_tests\amina_test.cpp'))
}

$Compiler = $null
if (Get-Command g++ -ErrorAction SilentlyContinue) { $Compiler = 'g++' }
elseif (Get-Command cl -ErrorAction SilentlyContinue) { $Compiler = 'cl' }

function Build-One([string]$key) {
    $out = $Exe[$key]
    $srcs = $Sources[$key]
    $newest = ($srcs + @(Get-ChildItem $SrcDir -Filter *.h | ForEach-Object { $_.FullName })) |
              ForEach-Object { (Get-Item $_).LastWriteTime } | Sort-Object -Descending | Select-Object -First 1
    if (-not $Build -and (Test-Path $out) -and ((Get-Item $out).LastWriteTime -gt $newest)) { return $true }
    if (-not $Compiler) { return $false }
    Write-Host "  compiling $(Split-Path $out -Leaf) with $Compiler ..."
    if (Test-Path $out) { Remove-Item $out -Force }
    if ($Compiler -eq 'g++') {
        & g++ -std=c++17 -O2 -I $SrcDir -o $out @srcs 2>&1 | ForEach-Object { "    $_" } | Out-Host
    } else {
        $obj = Join-Path $BuildDir ('obj_' + $key)
        New-Item -ItemType Directory -Force -Path $obj | Out-Null
        Push-Location $obj
        & cl /nologo /std:c++17 /EHsc /utf-8 /I $SrcDir ('/Fe' + $out) @srcs 2>&1 | ForEach-Object { "    $_" } | Out-Host
        Pop-Location
    }
    return (Test-Path $out)
}

Write-Host "`n=== Build ===" -ForegroundColor Cyan
if (-not $Compiler -and -not (Test-Path $Exe['atm'])) {
    Write-Host 'No compiler found. Install MinGW-w64 (g++) and add it to PATH, or run this script from' -ForegroundColor Red
    Write-Host '"x64 Native Tools Command Prompt for VS" (type: powershell -ExecutionPolicy Bypass -File tests\amina_tests\run_all_tests.ps1).' -ForegroundColor Red
    exit 2
}
$Built = @{}
foreach ($k in @('atm', 'bank', 'persd', 'amina')) {
    $Built[$k] = Build-One $k
    if (-not $Built[$k]) { Write-Host "  BUILD FAILED: $(Split-Path $Exe[$k] -Leaf)" -ForegroundColor Red }
}
if (-not $Built['atm']) { Write-Host 'atm.exe did not build. Fix the compile errors above first.' -ForegroundColor Red; exit 1 }
Write-Host "  using $($Exe['atm'])"

# ---------------------------------------------------------------- process runner
# Start-Process with file redirection passes the bytes through unchanged (no encoding or BOM problems).
$script:RunCounter = 0
function Invoke-Prog([string]$exePath, [string[]]$argList, [string[]]$lines, [string]$cwd, [string]$stdinFile) {
    $script:RunCounter++
    $tag = 'r' + $script:RunCounter
    $inF  = Join-Path $TmpDir "$tag.in"
    $outF = Join-Path $TmpDir "$tag.out"
    $errF = Join-Path $TmpDir "$tag.err"
    if ($stdinFile) { Copy-Item $stdinFile $inF -Force }
    else {
        $sb = New-Object System.Text.StringBuilder
        foreach ($l in $lines) { [void]$sb.Append($l).Append("`n") }
        [IO.File]::WriteAllText($inF, $sb.ToString(), $Utf8)
    }
    $sp = @{ FilePath = $exePath; WorkingDirectory = $cwd; RedirectStandardInput = $inF;
             RedirectStandardOutput = $outF; RedirectStandardError = $errF; NoNewWindow = $true; PassThru = $true }
    if ($argList -and $argList.Count -gt 0) { $sp['ArgumentList'] = @($argList | ForEach-Object { '"' + $_ + '"' }) }
    $p = Start-Process @sp
    $null = $p.Handle
    $timedOut = $false
    if (-not $p.WaitForExit(20000)) { $timedOut = $true; try { $p.Kill() } catch {} ; Start-Sleep -Milliseconds 300 }
    $o = ''; $e = ''
    for ($i = 0; $i -lt 10; $i++) {
        try { $o = [IO.File]::ReadAllText($outF, $Utf8); $e = [IO.File]::ReadAllText($errF, $Utf8); break }
        catch { Start-Sleep -Milliseconds 100 }
    }
    Remove-Item $inF, $outF, $errF -Force -ErrorAction SilentlyContinue
    return @{ Out = ($o -replace "`r`n", "`n"); Err = ($e -replace "`r`n", "`n"); TimedOut = $timedOut }
}

# Where the program stopped to wait for input k+1 = output of a run with only k inputs,
# minus the end-of-input tail ("Goodbye." and a summary printed because input ran out).
function Get-Boundary([string]$s) {
    if ($s.EndsWith("Goodbye.`n")) { $s = $s.Substring(0, $s.Length - 9) }
    $m = [regex]::Match($s, '\n=== (Session Summary|거래 요약) ===\n((Card|카드): [^\n]*\n)?(\[ID [^\n]*\n)+\z')
    if ($m.Success) { $s = $s.Substring(0, $m.Index) }
    return $s.Length
}

# Rebuilds what the console looks like when a person types the inputs (each input after its prompt).
function Get-EchoTranscript([string]$data, [string[]]$lines, [string]$cwd, [string]$final) {
    $pos = 0
    $sb = New-Object System.Text.StringBuilder
    for ($k = 0; $k -lt $lines.Count; $k++) {
        $prefix = @()
        if ($k -gt 0) { $prefix = $lines[0..($k - 1)] }
        $r = Invoke-Prog $Exe['atm'] @($data) $prefix $cwd $null
        $b = Get-Boundary $r.Out
        if ($b -lt $pos -or $b -gt $final.Length -or $final.Substring(0, $b) -cne $r.Out.Substring(0, $b)) { return $null }
        [void]$sb.Append($final.Substring($pos, $b - $pos)).Append($lines[$k]).Append("`n")
        $pos = $b
    }
    [void]$sb.Append($final.Substring($pos))
    return $sb.ToString()
}

# ---------------------------------------------------------------- test tables
$Tests = @(
    @{ Id = 'R01_initial_snapshot'; Cat = 'Setup'; Req = 'REQ1.1, 1.2, 1.4, 1.5, 1.10, 1.11, 10.1'; Data = 'main';
       Why = 'Typing / right after start shows every ATM (6-digit serial, bills per denomination) and every account exactly as written in the input file, so the file was loaded and nothing is hard-coded.';
       In = @('/', '0');
       Expect = @('ATM [SN: 000111] remaining cash: {KRW 50000 : 2, KRW 10000 : 3, KRW 5000 : 1, KRW 1000 : 3}, total: KRW 138000', 'ATM [SN: 000222] remaining cash: {KRW 50000 : 0, KRW 10000 : 0, KRW 5000 : 0, KRW 1000 : 60}, total: KRW 60000', 'Owner: Chris, balance: KRW 4000', 'Owner: Andor, balance: KRW 6000', 'Single-Bank, Unilingual', 'Multi-Bank, Bilingual');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'R02_multiple_accounts_and_banks'; Cat = 'Setup'; Req = 'REQ1.6, 1.7, 1.11'; Data = 'ext';
       Why = 'A different input file with Chris owning two Kakao accounts and one IM account, plus a third ATM, loads without code changes. This shows one user can have several accounts in one bank and accounts in several banks.';
       In = @('/', '0');
       Expect = @('No: 100-100-333333] Owner: Chris', 'Bank: IM, No: 200-200-444444] Owner: Chris', 'ATM [SN: 000333]');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'R03_broken_input_file'; Cat = 'Setup'; Req = 'REQ1.11, 9.1'; Data = 'broken';
       Why = 'An input file with a 5-digit serial number is rejected with the line number instead of crashing, so bad file input is handled.';
       In = @();
       Expect = @('Failed to load initial conditions', 'serial number must be exactly 6 digits');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'TC1_sample_single_deposit'; Cat = 'Sample routine'; Req = 'REQ10.1, 4.1, 4.3, 4.5'; Data = 'main';
       Why = 'This is Test Case 1 from the sample test routine: snapshot, cash deposit of 4 x 1,000 to Chris on ATM 1, then a second snapshot. Expected: Chris 4,000 to 8,000 and ATM 1 1,000-bills 3 to 7.';
       In = @('/', '1', '1111-1111-1111', '1234', '1', '1', '0', '0', '0', '4', '4', '/', '0');
       Expect = @('ATM [SN: 000111] remaining cash: {KRW 50000 : 2, KRW 10000 : 3, KRW 5000 : 1, KRW 1000 : 3}, total: KRW 138000', 'Owner: Chris, balance: KRW 4000', 'KRW 1000 : 7}, total: KRW 142000', 'Owner: Chris, balance: KRW 8000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'S01_two_sessions_unique_ids'; Cat = 'Session'; Req = 'REQ2.1, 2.2, 2.3, 2.4'; Data = 'main';
       Why = 'Two sessions in one program run. Session 1 does a deposit and a withdrawal and is cancelled, so its summary is printed. Session 2 continues with ID 3, so IDs are unique across sessions.';
       In = @('1', '1111-1111-1111', '1234', '1', '1', '0', '0', '0', '4', '2', '2000', '4', '1', '1111-1111-1111', '1234', '1', '2', '100000', '4', '0');
       Expect = @('=== Session Summary ===', 'Card: 1111-1111-1111 | Account: 100-100-111111', '[ID 1] Cash Deposit', '[ID 2] Cash Withdrawal', '[ID 3] Check Deposit');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'S02_no_summary_when_nothing_done'; Cat = 'Session'; Req = 'REQ2.3'; Data = 'main';
       Why = 'The user logs in and cancels straight away. No transaction succeeded, so no summary is printed (the spec allows this).';
       In = @('1', '1111-1111-1111', '1234', '4', '0');
       Expect = @('Session ended. Please take your card.');
       NotExpect = @('Session Summary'); File = $null; FileText = $null },
    @{ Id = 'S03_single_bank_rejects_other_bank'; Cat = 'Authorization'; Req = 'REQ1.2, 3.1, 3.2'; Data = 'main';
       Why = 'The IM card is inserted into the Kakao single-bank ATM. It must be rejected before the password is asked.';
       In = @('1', '2222-2222-2222', '0');
       Expect = @('This card cannot be used on this ATM.');
       NotExpect = @('Enter password'); File = $null; FileText = $null },
    @{ Id = 'S04_multi_bank_accepts_other_bank'; Cat = 'Authorization'; Req = 'REQ1.2, 3.1'; Data = 'main';
       Why = 'The Kakao card is inserted into the IM multi-bank ATM. It is accepted and the password prompt appears.';
       In = @('2', '1111-1111-1111', '1', '1234', '4', '0');
       Expect = @('Enter password: ', '1. Deposit');
       NotExpect = @('This card cannot be used'); File = $null; FileText = $null },
    @{ Id = 'S05_unknown_card'; Cat = 'Authorization'; Req = 'REQ3.2'; Data = 'main';
       Why = 'A card number that no bank issued is rejected with an error message.';
       In = @('1', '9999-9999-9999', '0');
       Expect = @('This card cannot be used on this ATM.');
       NotExpect = @('Enter password'); File = $null; FileText = $null },
    @{ Id = 'S06_wrong_password_once'; Cat = 'Authorization'; Req = 'REQ3.3, 3.4'; Data = 'main';
       Why = 'One wrong password shows Wrong Password. The correct one then opens the menu, so the bank really checks the password.';
       In = @('1', '1111-1111-1111', '9999', '1234', '4', '0');
       Expect = @('Wrong password. Please try again.', '1. Deposit');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'S07_wrong_password_three_times'; Cat = 'Authorization'; Req = 'REQ3.5'; Data = 'main';
       Why = 'Three wrong passwords in a row abort the session and the card is returned. The transaction menu never appears.';
       In = @('1', '1111-1111-1111', '0000', '1111', '2222', '0');
       Expect = @('Too many wrong password attempts', 'Session ended. Please take your card.');
       NotExpect = @('1. Deposit'); File = $null; FileText = $null },
    @{ Id = 'D01_cash_deposit_primary'; Cat = 'Deposit'; Req = 'REQ4.1, 4.3, 4.5, 1.8'; Data = 'main';
       Why = 'Cash deposit to the primary bank: the fee is 0, the account goes up by the amount, and the ATM gets the bills.';
       In = @('1', '1111-1111-1111', '1234', '1', '1', '0', '0', '0', '4', '4', '/', '0');
       Expect = @('Deposit completed.', 'Owner: Chris, balance: KRW 8000', 'KRW 1000 : 7}');
       NotExpect = @('Deposit fee required'); File = $null; FileText = $null },
    @{ Id = 'D02_cash_deposit_nonprimary_fee'; Cat = 'Deposit'; Req = 'REQ4.4, 4.5, 1.8'; Data = 'main';
       Why = 'Kakao card on the IM ATM: a 2,000 fee is asked for separately in cash. The account gets 5,000 and the ATM gets 5 + 2 bills.';
       In = @('2', '1111-1111-1111', '1', '1234', '1', '1', '0', '0', '0', '5', '0', '0', '0', '2', '4', '/', '0');
       Expect = @('Deposit fee required - insert it in cash (KRW): 2000', 'Owner: Chris, balance: KRW 9000', 'KRW 1000 : 67}');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'D03_fee_mismatch_ends_session'; Cat = 'Deposit'; Req = 'REQ4.4, 9.1'; Data = 'main';
       Why = 'A fee of 1,000 instead of 2,000 is an error. The session ends and no money moves.';
       In = @('2', '1111-1111-1111', '1', '1234', '1', '1', '0', '0', '0', '5', '0', '0', '0', '1', '/', '0');
       Expect = @('the inserted fee does not match', 'Owner: Chris, balance: KRW 4000', 'KRW 1000 : 60}');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'D04_more_than_50_bills'; Cat = 'Deposit'; Req = 'REQ4.2, 9.1'; Data = 'main';
       Why = '60 bills is over the 50-bill limit. Error, and the balance is unchanged.';
       In = @('1', '1111-1111-1111', '1234', '1', '1', '0', '0', '30', '30', '/', '0');
       Expect = @('too many bills (maximum 50', 'Owner: Chris, balance: KRW 4000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'D05_exactly_50_bills'; Cat = 'Deposit'; Req = 'REQ4.2'; Data = 'main';
       Why = 'This is a boundary value: exactly 50 bills is still allowed.';
       In = @('1', '1111-1111-1111', '1234', '1', '1', '0', '0', '0', '50', '4', '/', '0');
       Expect = @('Deposit completed.', 'Owner: Chris, balance: KRW 54000', 'KRW 1000 : 53}');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'D06_check_deposit_no_atm_cash'; Cat = 'Deposit'; Req = 'REQ4.1, 4.3, 4.6'; Data = 'main';
       Why = 'A check of 150,000 raises the balance, but the ATM cash stays at 138,000.';
       In = @('1', '1111-1111-1111', '1234', '1', '2', '150000', '4', '/', '0');
       Expect = @('Check Deposit', 'Owner: Chris, balance: KRW 154000', 'ATM [SN: 000111] remaining cash: {KRW 50000 : 2, KRW 10000 : 3, KRW 5000 : 1, KRW 1000 : 3}, total: KRW 138000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'D07_check_exactly_100000'; Cat = 'Deposit'; Req = 'REQ4.1, 1.10'; Data = 'main';
       Why = 'This is a boundary value: a check of exactly 100,000 is valid.';
       In = @('1', '1111-1111-1111', '1234', '1', '2', '100000', '4', '/', '0');
       Expect = @('Deposit completed.', 'Owner: Chris, balance: KRW 104000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'D08_check_below_minimum'; Cat = 'Deposit'; Req = 'REQ1.10, 9.1'; Data = 'main';
       Why = 'A check of 99,999 is below the minimum, so it is rejected.';
       In = @('1', '1111-1111-1111', '1234', '1', '2', '99999', '/', '0');
       Expect = @('invalid check (minimum KRW 100,000)', 'Owner: Chris, balance: KRW 4000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'D09_check_nonprimary_fee_in_cash'; Cat = 'Deposit'; Req = 'REQ4.4'; Data = 'main';
       Why = 'Check deposit at a non-primary ATM: the 2,000 fee is paid in cash. The account gets the check amount and the ATM gets only the 2 fee bills.';
       In = @('2', '1111-1111-1111', '1', '1234', '1', '2', '100000', '0', '0', '0', '2', '4', '/', '0');
       Expect = @('Owner: Chris, balance: KRW 104000', 'KRW 1000 : 62}');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'D10_no_cash_inserted'; Cat = 'Deposit'; Req = 'REQ4.1, 9.1'; Data = 'main';
       Why = 'A deposit of 0 bills is invalid input.';
       In = @('1', '1111-1111-1111', '1234', '1', '1', '0', '0', '0', '0', '/', '0');
       Expect = @('no cash was inserted');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'W01_fewest_bills_17000'; Cat = 'Withdrawal'; Req = 'REQ5.1, 5.3, 5.4, 5.5'; Data = 'main';
       Why = 'The spec''s own example: 17,000 is paid as 1 x 10,000 + 1 x 5,000 + 2 x 1,000. The account pays 17,000 + 2,000 fee and the ATM loses those bills.';
       In = @('1', '1111-1111-1111', '1234', '1', '2', '100000', '2', '17000', '4', '/', '0');
       Expect = @('KRW 10000 x 1', 'KRW 5000 x 1', 'KRW 1000 x 2', 'Owner: Chris, balance: KRW 85000', 'KRW 10000 : 2, KRW 5000 : 0, KRW 1000 : 1}, total: KRW 121000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'W02_insufficient_funds'; Cat = 'Withdrawal'; Req = 'REQ5.2, 9.1'; Data = 'main';
       Why = '3,000 + 2,000 fee is more than Chris''s 4,000. Error, and nothing changes.';
       In = @('1', '1111-1111-1111', '1234', '2', '3000', '/', '0');
       Expect = @('insufficient funds in the account', 'Owner: Chris, balance: KRW 4000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'W03_insufficient_atm_cash'; Cat = 'Withdrawal'; Req = 'REQ5.2, 9.1'; Data = 'main';
       Why = 'The account has enough money but ATM 1 has only 138,000. Error, and nothing changes.';
       In = @('1', '1111-1111-1111', '1234', '1', '2', '400000', '2', '200000', '/', '0');
       Expect = @('insufficient cash in the ATM', 'Owner: Chris, balance: KRW 404000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'W04_over_500000'; Cat = 'Withdrawal'; Req = 'REQ5.7'; Data = 'main';
       Why = '600,000 is over the 500,000 per-transaction limit.';
       In = @('1', '1111-1111-1111', '1234', '1', '2', '1000000', '2', '600000', '/', '0');
       Expect = @('maximum withdrawal per transaction is KRW 500,000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'W05_exactly_500000'; Cat = 'Withdrawal'; Req = 'REQ5.7, 5.1'; Data = 'ext';
       Why = 'This is a boundary value: 500,000 is allowed, paid as 10 x 50,000 on an ATM that has the bills.';
       In = @('3', '1111-1111-1111', '1234', '1', '2', '1000000', '2', '500000', '4', '/', '0');
       Expect = @('KRW 50000 x 10', 'Withdrawal completed.');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'W06_fourth_withdrawal_refused'; Cat = 'Withdrawal'; Req = 'REQ5.6'; Data = 'main';
       Why = 'Three withdrawals succeed and the fourth one in the same session is refused.';
       In = @('1', '1111-1111-1111', '1234', '1', '2', '1000000', '2', '1000', '2', '1000', '2', '1000', '2', '/', '0');
       Expect = @('[ID 4] Cash Withdrawal', 'maximum number of withdrawals per session (3)');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'W07_nonprimary_fee_3000'; Cat = 'Withdrawal'; Req = 'REQ5.4, 1.8'; Data = 'main';
       Why = 'Kakao card on the IM ATM pays a 3,000 withdrawal fee: 4,000 + 10,000 deposited - 5,000 - 3,000 = 6,000.';
       In = @('2', '1111-1111-1111', '1', '1234', '1', '1', '0', '0', '0', '10', '0', '0', '0', '2', '2', '5000', '4', '/', '0');
       Expect = @('Fee charged to the account (KRW): 3000', 'Owner: Chris, balance: KRW 6000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'W08_not_multiple_of_1000'; Cat = 'Withdrawal'; Req = 'REQ5.1, 9.1'; Data = 'main';
       Why = '2,500 cannot be paid with the available bill types.';
       In = @('1', '1111-1111-1111', '1234', '1', '2', '100000', '2', '2500', '/', '0');
       Expect = @('multiple of KRW 1,000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'W09_dispense_over_50_bills'; Cat = 'Withdrawal'; Req = 'REQ5.2, 4.2'; Data = 'main';
       Why = 'ATM 2 has only 1,000-bills, so 60,000 would need 60 bills. That is over the 50-bill limit, so it is refused.';
       In = @('2', '2222-2222-2222', '1', '4321', '1', '2', '100000', '2', '60000', '/', '0');
       Expect = @('exceed 50 bills', 'ATM [SN: 000222] remaining cash: {KRW 50000 : 0, KRW 10000 : 0, KRW 5000 : 0, KRW 1000 : 60}, total: KRW 60000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'T01_cash_transfer'; Cat = 'Transfer'; Req = 'REQ6.1, 6.2, 6.3, 6.5, 6.6, 6.7'; Data = 'main';
       Why = 'Chris inserts one 5,000 bill for Andor. The ATM shows the amount to transfer without the fee (4,000) and asks to confirm. Andor gets 4,000 and the 5,000 bill is added to ATM 1.';
       In = @('1', '1111-1111-1111', '1234', '3', '1', '200-200-222222', '0', '0', '1', '0', '1', '4', '/', '0');
       Expect = @('Amount to transfer: KRW 4,000 (Fee (KRW): 1000)', 'Fee (KRW): 1000', 'Transfer completed.', 'Owner: Andor, balance: KRW 10000', 'KRW 5000 : 2', 'Owner: Chris, balance: KRW 4000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'T02_account_transfer_primary_to_nonprimary'; Cat = 'Transfer'; Req = 'REQ6.4, 6.5, 6.7, 1.8'; Data = 'main';
       Why = 'Kakao to IM on the Kakao ATM: the fee is 3,000. Chris 4,000 - 1,000 - 3,000 = 0 and Andor gets 1,000. ATM cash does not change.';
       In = @('1', '1111-1111-1111', '1234', '3', '2', '200-200-222222', '1000', '4', '/', '0');
       Expect = @('Fee (KRW): 3000', 'Owner: Chris, balance: KRW 0', 'Owner: Andor, balance: KRW 7000', 'ATM [SN: 000111] remaining cash: {KRW 50000 : 2, KRW 10000 : 3, KRW 5000 : 1, KRW 1000 : 3}, total: KRW 138000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'T03_account_transfer_primary_to_primary'; Cat = 'Transfer'; Req = 'REQ6.5, 1.8'; Data = 'ext';
       Why = 'Kakao to Kakao on the Kakao ATM: the fee is 2,000.';
       In = @('1', '3333-3333-3333', '1111', '3', '2', '100-100-111111', '10000', '4', '/', '0');
       Expect = @('Fee (KRW): 2000', 'No: 100-100-333333] Owner: Chris, balance: KRW 488000', 'No: 100-100-111111] Owner: Chris, balance: KRW 14000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'T04_account_transfer_nonprimary_to_nonprimary'; Cat = 'Transfer'; Req = 'REQ6.5, 1.8'; Data = 'ext';
       Why = 'Kakao to Kakao on the IM ATM: both banks are non-primary, so the fee is 5,000.';
       In = @('2', '3333-3333-3333', '1', '1111', '3', '2', '100-100-111111', '10000', '4', '/', '0');
       Expect = @('Fee (KRW): 5000', 'No: 100-100-333333] Owner: Chris, balance: KRW 485000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'T05_account_transfer_nonprimary_to_primary'; Cat = 'Transfer'; Req = 'REQ6.5, 1.8'; Data = 'ext';
       Why = 'Kakao to IM on the IM ATM: one bank is primary, so the fee is 3,000.';
       In = @('2', '3333-3333-3333', '1', '1111', '3', '2', '200-200-222222', '10000', '4', '/', '0');
       Expect = @('Fee (KRW): 3000', 'No: 100-100-333333] Owner: Chris, balance: KRW 487000', 'Owner: Andor, balance: KRW 16000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'T06_account_transfer_insufficient'; Cat = 'Transfer'; Req = 'REQ6.7, 9.1'; Data = 'main';
       Why = '3,000 + 3,000 fee is more than Chris''s 4,000. Error, and both balances are unchanged.';
       In = @('1', '1111-1111-1111', '1234', '3', '2', '200-200-222222', '3000', '/', '0');
       Expect = @('Insufficient funds in your account.', 'Owner: Chris, balance: KRW 4000', 'Owner: Andor, balance: KRW 6000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'T07_transfer_unknown_destination'; Cat = 'Transfer'; Req = 'REQ6.2, 9.1'; Data = 'main';
       Why = 'The destination account does not exist.';
       In = @('1', '1111-1111-1111', '1234', '3', '2', '999-999-999999', '/', '0');
       Expect = @('Account not found.');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'T09_cash_transfer_cancelled'; Cat = 'Transfer'; Req = 'REQ6.3, 9.1'; Data = 'main';
       Why = 'The user cancels at the confirmation step. Nothing is transferred and the inserted bill is not kept by the ATM.';
       In = @('1', '1111-1111-1111', '1234', '3', '1', '200-200-222222', '0', '0', '1', '0', '2', '/', '0');
       Expect = @('Transfer cancelled.', 'Owner: Andor, balance: KRW 6000', 'ATM [SN: 000111] remaining cash: {KRW 50000 : 2, KRW 10000 : 3, KRW 5000 : 1, KRW 1000 : 3}, total: KRW 138000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'T08_cash_transfer_only_fee'; Cat = 'Transfer'; Req = 'REQ6.3, 9.1'; Data = 'main';
       Why = 'Only 1,000 is inserted, which is just the fee, so there is nothing to transfer.';
       In = @('1', '1111-1111-1111', '1234', '3', '1', '200-200-222222', '0', '0', '0', '1', '/', '0');
       Expect = @('must be more than 1,000 KRW fee', 'Owner: Andor, balance: KRW 6000');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'A01_admin_history_and_file'; Cat = 'Admin'; Req = 'REQ1.9, 7.1, 7.2, 7.3'; Data = 'main';
       Why = 'After a deposit, a withdrawal and a transfer, the admin card shows only the Transaction History menu. It lists every transaction with its ID, card, type and amount, and writes the list to a .txt file.';
       In = @('1', '1111-1111-1111', '1234', '1', '2', '200000', '2', '50000', '3', '2', '200-200-222222', '1000', '4', '1', '0000', '1', '2', '0');
       Expect = @('[Admin Menu]', '1. View Transaction History', '=== Transaction History ===', '[ID 1] Check Deposit | Card: 1111-1111-1111 | Amount: 200000 KRW', '[ID 2] Cash Withdrawal | Card: 1111-1111-1111 | Amount: 50000 KRW', '[ID 3] Account Transfer | Card: 1111-1111-1111 | Amount: 1000 KRW', 'saved to file: Kakao ATM 000111 transac_history.txt');
       NotExpect = @(); File = 'Kakao ATM 000111 transac_history.txt'; FileText = '[ID 3] Account Transfer' },
    @{ Id = 'A02_admin_no_history'; Cat = 'Admin'; Req = 'REQ7.2'; Data = 'main';
       Why = 'The admin card used before any transaction says the history is empty.';
       In = @('1', '0000', '1', '2', '0');
       Expect = @('No transactions recorded yet.');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'A03_admin_failed_tx_not_logged'; Cat = 'Admin'; Req = 'REQ7.2'; Data = 'main';
       Why = 'A failed withdrawal is not recorded. Only successful transactions appear in the history.';
       In = @('1', '1111-1111-1111', '1234', '2', '3000', '1', '0000', '1', '2', '0');
       Expect = @('No transactions recorded yet.');
       NotExpect = @('Cash Withdrawal |'); File = $null; FileText = $null },
    @{ Id = 'K01_unilingual_never_asks'; Cat = 'Language'; Req = 'REQ1.3'; Data = 'main';
       Why = 'The unilingual ATM shows English only and never asks for a language.';
       In = @('1', '1111-1111-1111', '1234', '4', '0');
       Expect = @('1. Deposit');
       NotExpect = @('Select language'); File = $null; FileText = $null },
    @{ Id = 'K02_korean_menus'; Cat = 'Language'; Req = 'REQ1.3, 8.1, 8.2'; Data = 'main';
       Why = 'The bilingual ATM offers English/Korean. After choosing Korean, the password prompt, menus and messages are in Korean.';
       In = @('2', '2222-2222-2222', '2', '4321', '1', '1', '0', '0', '0', '1', '4', '0');
       Expect = @('언어를 선택하세요', '비밀번호를 입력하세요', '1. 입금', '입금이 완료되었습니다.', '거래가 종료되었습니다.');
       NotExpect = @('1. Deposit'); File = $null; FileText = $null },
    @{ Id = 'K03_korean_error_message'; Cat = 'Language'; Req = 'REQ8.2, 9.1'; Data = 'main';
       Why = 'An error inside a Korean session is shown in Korean.';
       In = @('2', '1111-1111-1111', '2', '1234', '1', '2', '99999', '/', '0');
       Expect = @('유효하지 않은 수표입니다');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'K04_korean_summary_fully_korean'; Cat = 'Language'; Req = 'REQ8.2'; Data = 'main';
       Why = 'KNOWN BUG CHECK: the session summary in a Korean session should not contain English words. This currently FAILS because type() and describe() return hard-coded English text.';
       In = @('2', '2222-2222-2222', '2', '4321', '1', '1', '0', '0', '0', '1', '4', '0');
       Expect = @('=== 거래 요약 ===');
       NotExpect = @('Cash Deposit', 'Won added'); File = $null; FileText = $null },
    @{ Id = 'K05_korean_transfer_summary'; Cat = 'Language'; Req = 'REQ8.2, 6.7'; Data = 'main';
       Why = 'An account transfer in a Korean session: the summary line, including the transfer type and amount, is fully Korean.';
       In = @('2', '2222-2222-2222', '2', '4321', '3', '2', '100-100-111111', '1000', '4', '0');
       Expect = @('[ID 1] 계좌 이체 - 1,000원 200-200-222222 -> 100-100-111111');
       NotExpect = @('Account Transfer', 'Won'); File = $null; FileText = $null },
    @{ Id = 'A04_admin_history_korean'; Cat = 'Admin'; Req = 'REQ7.2, 8.2'; Data = 'main';
       Why = 'The admin chooses Korean on the bilingual ATM. The history labels are Korean.';
       In = @('2', '2222-2222-2222', '2', '4321', '3', '2', '100-100-111111', '1000', '4', '2', '0000', '2', '1', '2', '0');
       Expect = @('세션 #1 (카드: 2222-2222-2222)', '| 금액: 1000원');
       NotExpect = @('Session #', 'Amount:'); File = $null; FileText = $null },
    @{ Id = 'X01_non_numeric_menu_input'; Cat = 'Exceptions'; Req = 'REQ9.1'; Data = 'main';
       Why = 'Letters and out-of-range numbers at the menu show Invalid input instead of crashing.';
       In = @('1', '1111-1111-1111', '1234', 'abc', '9', '4', '0');
       Expect = @('Invalid input.', 'Session ended. Please take your card.');
       NotExpect = @(); File = $null; FileText = $null },
    @{ Id = 'X02_snapshot_inside_transaction'; Cat = 'Snapshot'; Req = 'REQ10.1'; Data = 'main';
       Why = '/ typed in the middle of a withdrawal prints the snapshot, then the same prompt continues.';
       In = @('1', '1111-1111-1111', '1234', '2', '/', '1000', '4', '0');
       Expect = @('==================== SNAPSHOT', 'Withdrawal completed.');
       NotExpect = @(); File = $null; FileText = $null }
)

$Harness = @(
    @{ Id = 'tc01_cash_deposit_primary'; Req = 'REQ4.1, 4.3, 4.5'; Why = 'Primary-bank cash deposit: fee 0, balance +4,000.'; Args = @('deposit', '1', '1'); Expect = @('Deposit completed.', 'Owner: Chris, balance: KRW 8000') },
    @{ Id = 'tc02_cash_deposit_nonprimary_with_fee'; Req = 'REQ4.4, 1.8'; Why = 'Non-primary cash deposit with the 2,000 fee inserted separately.'; Args = @('deposit', '2', '1'); Expect = @('Deposit completed.', 'Owner: Chris, balance: KRW 9000') },
    @{ Id = 'tc03_check_deposit_ok'; Req = 'REQ4.1, 4.6'; Why = 'Valid check of 150,000 is added to the account.'; Args = @('deposit', '1', '1'); Expect = @('Deposit completed.', 'Owner: Chris, balance: KRW 154000') },
    @{ Id = 'tc04_check_too_small'; Req = 'REQ1.10'; Why = 'Check of 99,999 is under the 100,000 minimum.'; Args = @('deposit', '1', '1'); Expect = @('invalid check') },
    @{ Id = 'tc05_wrong_fee'; Req = 'REQ4.4'; Why = 'Fee of 1,000 instead of 2,000 is rejected.'; Args = @('deposit', '2', '1'); Expect = @('the inserted fee does not match') },
    @{ Id = 'tc06_deposit_zero_cash'; Req = 'REQ4.1'; Why = 'Deposit of zero bills is rejected.'; Args = @('deposit', '1', '1'); Expect = @('no cash was inserted') },
    @{ Id = 'tc07_withdraw_primary_ok'; Req = 'REQ5.3, 5.4'; Why = 'Primary withdrawal of 2,000 + 2,000 fee empties the 4,000 account exactly (boundary).'; Args = @('withdraw', '1', '1'); Expect = @('Withdrawal completed.', 'Owner: Chris, balance: KRW 0') },
    @{ Id = 'tc08_withdraw_not_enough_funds'; Req = 'REQ5.2'; Why = '3,000 + 2,000 fee is more than the balance.'; Args = @('withdraw', '1', '1'); Expect = @('insufficient funds') },
    @{ Id = 'tc09_withdraw_nonprimary_fee3000'; Req = 'REQ5.4, 1.8'; Why = 'Non-primary withdrawal fee is 3,000.'; Args = @('withdraw', '2', '1'); Expect = @('Fee charged to the account (KRW): 3000') },
    @{ Id = 'tc10_withdraw_not_multiple_of_1000'; Req = 'REQ5.1'; Why = '2,500 is not a multiple of 1,000.'; Args = @('withdraw', '1', '1', '0', '100000'); Expect = @('multiple of KRW 1,000') },
    @{ Id = 'tc11_fourth_withdrawal_refused'; Req = 'REQ5.6'; Why = 'A 4th withdrawal (3 already done) is refused.'; Args = @('withdraw', '1', '1', '3', '100000'); Expect = @('maximum number of withdrawals') },
    @{ Id = 'tc12_withdraw_17000_fewest_bills'; Req = 'REQ5.1, 5.5'; Why = '17,000 is paid with the fewest bills (10k + 5k + 2 x 1k).'; Args = @('withdraw', '1', '1', '0', '100000'); Expect = @('KRW 10000 x 1', 'KRW 5000 x 1', 'KRW 1000 x 2') },
    @{ Id = 'tc13_withdraw_over_500000'; Req = 'REQ5.7'; Why = '600,000 is over the 500,000 limit.'; Args = @('withdraw', '1', '1', '0', '1000000'); Expect = @('maximum withdrawal per transaction') },
    @{ Id = 'tc14_withdraw_atm_has_not_enough_cash'; Req = 'REQ5.2'; Why = '500,000 is more than the 138,000 cash inside ATM 1.'; Args = @('withdraw', '1', '1', '0', '1000000'); Expect = @('insufficient cash in the ATM') }
)

if ($Only) {
    $Tests   = @($Tests   | Where-Object { $id = $_.Id; @($Only | Where-Object { $id -like "$_*" }).Count -gt 0 })
    $Harness = @($Harness | Where-Object { $id = $_.Id; @($Only | Where-Object { $id -like "$_*" }).Count -gt 0 })
}

$Results = New-Object System.Collections.ArrayList
function Add-Result($id, $cat, $req, $ok, $why, $problems) {
    $res = 'PASS'; if (-not $ok) { $res = 'FAIL' }
    [void]$Results.Add([pscustomobject]@{ Id = $id; Category = $cat; REQ = $req; Result = $res; Why = $why; Problems = ($problems -join ' ; ') })
    if ($ok) { Write-Host ("  PASS  " + $id) -ForegroundColor Green }
    else { Write-Host ("  FAIL  " + $id) -ForegroundColor Red; foreach ($p in $problems) { Write-Host ("        - " + $p) -ForegroundColor Yellow } }
}

# ---------------------------------------------------------------- unit tests
if (-not $Only) {
    Write-Host "`n=== Unit tests ===" -ForegroundColor Cyan
    $units = @(
        @{ Key = 'bank';  Id = 'U01_bank_side_test';  Req = 'REQ1.5, 1.6, 1.7, 1.9, 1.11, 3.3'; Ok = 'All bank-side tests passed.';
           Why = 'Rabia''s unit tests for Card, Account, Bank and SystemLoader (file parsing, duplicates, password check).' },
        @{ Key = 'persd'; Id = 'U02_person_d_test';   Req = 'REQ6.1, 6.5, 6.7, 7.2, 7.3, 8.1, 9.1';  Ok = 'All Person D tests passed.';
           Why = 'Alan''s unit tests for messages, exceptions, transfers, history and the admin menu.' }
    )
    foreach ($u in $units) {
        if (-not $Built[$u.Key]) { Add-Result $u.Id 'Unit' $u.Req $false $u.Why @('did not compile'); continue }
        $r = Invoke-Prog $Exe[$u.Key] @() @() $Root $null
        $text = $r.Err + $r.Out
        [IO.File]::WriteAllText((Join-Path $TranscriptDir ($u.Id + '.txt')), "# $($u.Id)  [$($u.Req)]`n# Why: $($u.Why)`n" + ('-' * 60) + "`n" + $text, $Utf8Bom)
        $probs = @(); if (-not $text.Contains($u.Ok)) { $probs += "did not print '$($u.Ok)'" }
        Add-Result $u.Id 'Unit' $u.Req ($probs.Count -eq 0) $u.Why $probs
    }
}

if ($Harness.Count -gt 0) {
    Write-Host "`n=== Amina's deposit / withdrawal harness (tests\amina_tests\*.in) ===" -ForegroundColor Cyan
    foreach ($h in $Harness) {
        $inFile = Join-Path $Root ("tests\amina_tests\" + $h.Id + ".in")
        if (-not $Built['amina'] -or -not (Test-Path $inFile)) { Add-Result $h.Id 'Harness' $h.Req $false $h.Why @('harness not built or .in file missing'); continue }
        $r = Invoke-Prog $Exe['amina'] $h.Args @() $Root $inFile
        $text = $r.Err + $r.Out
        [IO.File]::WriteAllText((Join-Path $TranscriptDir ($h.Id + '.txt')), "# $($h.Id)  [$($h.Req)]  amina_test $($h.Args -join ' ')`n# Why: $($h.Why)`n" + ('-' * 60) + "`n" + $text, $Utf8Bom)
        $probs = @(); foreach ($e in $h.Expect) { if (-not $text.Contains($e)) { $probs += "missing: $e" } }
        Add-Result $h.Id 'Harness' $h.Req ($probs.Count -eq 0) $h.Why $probs
    }
}

# ---------------------------------------------------------------- end-to-end tests
Write-Host "`n=== End-to-end tests on atm.exe (program restarted for every test) ===" -ForegroundColor Cyan
foreach ($t in $Tests) {
    $cwd = Join-Path $RunDir $t.Id
    if (Test-Path $cwd) { Remove-Item $cwd -Recurse -Force }
    New-Item -ItemType Directory -Force -Path $cwd | Out-Null
    $data = $DataFiles[$t.Data]
    $r = Invoke-Prog $Exe['atm'] @($data) $t.In $cwd $null
    $full = $r.Err + $r.Out
    $probs = @()
    if ($r.TimedOut) { $probs += 'program did not finish within 20 s' }
    foreach ($e in $t.Expect)    { if (-not $full.Contains($e)) { $probs += "missing: $e" } }
    foreach ($e in $t.NotExpect) { if ($full.Contains($e))      { $probs += "should not appear: $e" } }
    if ($t.File) {
        $fp = Join-Path $cwd $t.File
        if (-not (Test-Path $fp)) { $probs += "file not created: $($t.File)" }
        elseif (-not ([IO.File]::ReadAllText($fp, $Utf8)).Contains($t.FileText)) { $probs += "file does not contain: $($t.FileText)" }
        else { Copy-Item $fp (Join-Path $TranscriptDir ($t.Id + '__' + $t.File)) -Force }
    }
    $body = $null
    if (-not $NoEcho -and $t.In.Count -gt 0) { $body = Get-EchoTranscript $data $t.In $cwd $r.Out }
    if ($null -eq $body) { $body = $r.Out }
    if ($r.Err) { $body = $r.Err + $body }
    $res = 'PASS'; if ($probs.Count -gt 0) { $res = 'FAIL' }
    $header = "# $($t.Id)   [$($t.Req)]   result: $res`n" +
              "# Why this test is reasonable: $($t.Why)`n" +
              "# Input file: $(Split-Path $data -Leaf)`n" +
              "# Typed (one per Enter): $($t.In -join ' | ')`n" + ('-' * 60) + "`n"
    [IO.File]::WriteAllText((Join-Path $TranscriptDir ($t.Id + '.txt')), $header + $body, $Utf8Bom)
    Add-Result $t.Id $t.Cat $t.Req ($probs.Count -eq 0) $t.Why $probs
}

# ---------------------------------------------------------------- reports
$Results | Export-Csv -Path (Join-Path $OutDir 'results.csv') -NoTypeInformation -Encoding UTF8

$AllReqs = @('1.1','1.2','1.3','1.4','1.5','1.6','1.7','1.8','1.9','1.10','1.11',
             '2.1','2.2','2.3','2.4','3.1','3.2','3.3','3.4','3.5',
             '4.1','4.2','4.3','4.4','4.5','4.6','5.1','5.2','5.3','5.4','5.5','5.6','5.7',
             '6.1','6.2','6.3','6.4','6.5','6.6','6.7','7.1','7.2','7.3','8.1','8.2','9.1','10.1')
$md = New-Object System.Text.StringBuilder
[void]$md.Append("# Requirement check sheet (draft, generated $(Get-Date -Format 'yyyy-MM-dd HH:mm'))`n`n")
[void]$md.Append("| REQ | O/X | Tests (transcripts\<id>.txt) |`n| --- | --- | --- |`n")
foreach ($q in $AllReqs) {
    $hits = @($Results | Where-Object { (' ' + ($_.REQ -replace 'REQ', '') + ',') -match ('[ ,]' + [regex]::Escape($q) + ',') })
    if ($hits.Count -eq 0) { $mark = '? (no test)' }
    elseif (@($hits | Where-Object { $_.Result -eq 'FAIL' }).Count -gt 0) { $mark = 'check (a test FAILED)' }
    else { $mark = 'O' }
    $names = ($hits | ForEach-Object { $_.Id + ' (' + $_.Result + ')' }) -join ', '
    [void]$md.Append("| REQ$q | $mark | $names |`n")
}
[void]$md.Append("`n## Test list with explanations`n`n| Test | REQ | Result | Why this test is reasonable |`n| --- | --- | --- | --- |`n")
foreach ($x in $Results) { [void]$md.Append("| $($x.Id) | $($x.REQ) | $($x.Result) | $($x.Why) |`n") }
[IO.File]::WriteAllText((Join-Path $OutDir 'checklist.md'), $md.ToString(), $Utf8Bom)

Remove-Item $TmpDir -Recurse -Force -ErrorAction SilentlyContinue
$pass = @($Results | Where-Object { $_.Result -eq 'PASS' }).Count
$fail = @($Results | Where-Object { $_.Result -eq 'FAIL' }).Count
Write-Host ''
$color = 'Green'; if ($fail -gt 0) { $color = 'Yellow' }
Write-Host "=== $pass passed, $fail failed  (of $($Results.Count)) ===" -ForegroundColor $color
Write-Host "Transcripts : $TranscriptDir"
Write-Host "Check sheet : $(Join-Path $OutDir 'checklist.md')"
Write-Host "CSV         : $(Join-Path $OutDir 'results.csv')"
Write-Host "Screenshot one test with:  .\tests\amina_tests\run_all_tests.ps1 -Show <test id>"
if ($fail -gt 0) { exit 1 } else { exit 0 }
