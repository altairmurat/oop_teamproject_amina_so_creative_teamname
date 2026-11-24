# Demo Rehearsal Script

Use this walkthrough to rehearse the live demo with `data/initial_conditions.txt`.

## Before starting

1. Build cleanly from the repo root:
   - Linux / macOS / MinGW: `make clean && make`
   - Windows (VS Native Tools prompt):
     ```
     chcp 65001
     mkdir build
     cl /std:c++17 /EHsc /utf-8 /Fobuild\ /Fe:build\atm.exe src\*.cpp
     ```
2. Remove any leftover `*transac_history.txt` files in the project folder so the exported file is freshly created during the demo.
3. Run the program:
   ```
   ./atm data/initial_conditions.txt
   ```
   (Always restart the program between test runs so balances and ATM cash reset.)

## Initial state in `data/initial_conditions.txt`

- Banks: `Kakao`, `IM`
- Accounts:
  - Chris (`Kakao`): `100-100-111111`, balance `4,000`, card `1111-1111-1111`, PIN `1234`
  - Andor (`IM`): `200-200-222222`, balance `6,000`, card `2222-2222-2222`, PIN `4321`
- ATMs:
  - ATM 1: `000111` (`Kakao`, Single-Bank, Unilingual) — `{50k: 2, 10k: 3, 5k: 1, 1k: 3}` = `138,000`
  - ATM 2: `000222` (`IM`, Multi-Bank, Bilingual) — `{50k: 0, 10k: 0, 5k: 0, 1k: 60}` = `60,000`
- Admin card: `0000`

---

## Walkthrough steps

### 1. Startup snapshot (REQ 1.11, REQ 10.1)
- At `Select an ATM:`, type `/`
- Check output:
  - ATM `000111`: total `KRW 138000`
  - ATM `000222`: total `KRW 60000`
  - Chris: `KRW 4000`, Andor: `KRW 6000`

### 2. Single-bank card rejection (REQ 3.1, REQ 3.2)
- Select ATM `1` (`000111`, Kakao Single-Bank)
- Card: `2222-2222-2222` (Andor's IM card)
- Expected: `This card cannot be used on this ATM.` and session ends immediately.

### 3. Wrong password 3 times (REQ 3.3, REQ 3.4, REQ 3.5)
- Select ATM `1`
- Card: `1111-1111-1111` (Chris)
- Enter `0000`, then `1111`, then `2222`
- Expected: `Wrong password. Please try again.` x3, then `Too many wrong password attempts. Session ended.`

### 4. Session #1 on ATM 1 — Deposit, Check Deposit, Withdrawal, Summary (REQ 2.3, 4.x, 5.x)
- Select ATM `1`, card `1111-1111-1111`, PIN `1234`
- **Cash deposit**: `1` -> `1` -> bills `0`, `1`, `0`, `3` (`13,000` KRW, primary bank fee `0`)
  - Chris: `4,000 -> 17,000`; ATM 1: `138,000 -> 151,000`
- **Check deposit**: `1` -> `2` -> `100000` (primary bank fee `0`)
  - Chris: `17,000 -> 117,000`; ATM 1 cash unchanged (`151,000`)
- **Withdrawal**: `2` -> `10000`
  - Dispenses `KRW 10000 x 1`, fee `2,000` charged to account
  - Chris: `117,000 -> 105,000`; ATM 1: `151,000 -> 141,000`
- **End session**: `4` (Cancel)
  - Expected: `=== Session Summary ===` lists IDs `1`, `2`, `3`.

### 5. Session #2 on ATM 2 in Korean — Transfers & Exception handling (REQ 6.x, 8.1, 9.1)
- Select ATM `2` (`000222`, IM Multi-Bank, Bilingual)
- Card: `1111-1111-1111` (Chris, accepted because ATM 2 is Multi-Bank)
- Language: `2` (Korean) -> all prompts switch to Korean
- PIN: `1234`
- **Cash transfer (현금 이체)**: `3` -> `1` -> destination `200-200-222222` -> bills `0`, `1`, `0`, `0` (`10,000` KRW inserted)
  - Fee `1,000` deducted from inserted cash -> `9,000` transferred to Andor
  - Andor: `6,000 -> 15,000`; ATM 2: `60,000 -> 70,000`
- **Account transfer (계좌 이체)**: `3` -> `2` -> destination `200-200-222222` -> amount `5000`
  - Non-primary (Kakao) to primary (IM) on IM ATM -> fee `3,000`
  - Chris: `105,000 -> 97,000`; Andor: `15,000 -> 20,000`
- **Exception triggers immediate session end (REQ 9.1)**: `3` -> `2` -> destination `100-100-111111` (same account)
  - Expected: `같은 계좌로는 이체할 수 없습니다.` -> session ends immediately and prints Korean summary (`=== 거래 요약 ===`) for IDs `4` and `5`.

### 6. Final snapshot & Admin Menu (REQ 1.9, 7.1, 7.2, 10.1)
- At `Select an ATM:`, type `/`
  - Verify final balances: Chris `KRW 97000`, Andor `KRW 20000`
  - Verify ATM cash: ATM `000111` = `KRW 141000`, ATM `000222` = `KRW 70000`
- Select ATM `2`, card `0000` (Admin), language `1` (English)
- Choose `1` (`View Transaction History`)
  - Expected: displays `Session #1 (Card: 1111-1111-1111)` (IDs 1-3) and `Session #2 (Card: 1111-1111-1111)` (IDs 4-5)
  - Prints `Transaction history saved to file: IM ATM 000222 transac_history.txt`
- Choose `2` (Exit Admin Menu), then `0` (Exit program).
- Open `IM ATM 000222 transac_history.txt` to show the exported file matches the console output.

## Quick automated check before demo day

Run both unit test suites to verify nothing broke after the last pull:
```bash
make test-bank
g++ -std=c++17 -Wall -Wextra -Isrc tests/person_d_test.cpp $(ls src/*.cpp | grep -v main.cpp) -o tests/_tmp_person_d_test && ./tests/_tmp_person_d_test && rm -f tests/_tmp_person_d_test
```
