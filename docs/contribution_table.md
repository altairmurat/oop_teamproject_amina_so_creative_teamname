# Team Contribution Table

Course: CSE201 Object-Oriented Programming (ATM Term Project)  
Team: Rabia, Altair, Amina, Alan

| Member | Role | Owned Files | Main Responsibilities | Contribution |
| --- | --- | --- | --- | --- |
| Rabia | Person A (Bank Side & Integration Lead) | `src/Bank.*`, `src/Account.*`, `src/Card.*`, `src/SystemLoader.*`, `src/Snapshot.*`, `src/main.cpp`, `data/initial_conditions.txt`, `Makefile`, `CMakeLists.txt`, `README.md`, `tests/bank_side_test.cpp` | Bank, Account, and Card/AdminCard hierarchy; initial condition file loader and validation; `/` snapshot command; main ATM selection loop; build scripts and run instructions; integration checks on `main`; bank-side unit tests | 25% |
| Altair | Person B (ATM Core, Session & UI) | `src/Constants.h`, `src/CashInventory.*`, `src/ATM.*`, `src/SingleBankATM.*`, `src/MultiBankATM.*`, `src/ConsoleUI.*`, `src/Session.*`, `docs/class_diagram.md`, `docs/oop_concepts.md` | ATM base class with Single-Bank and Multi-Bank subclasses; CashInventory with greedy fewest-bill dispensing; ConsoleUI input validation and `/` snapshot hook; Session lifecycle, card/PIN verification, and language switching; UML class diagram and OOP concepts doc | 25% |
| Amina | Person C (Transactions, Fees & QA) | `src/Transaction.*`, `src/FeePolicy.*`, `src/DepositTransaction.*`, `src/WithdrawalTransaction.*`, `tests/amina_tests/*` | Abstract Transaction base class with unique ID assignment; FeePolicy rules; Cash and Check Deposit (50-bill limit, min 100,000 KRW check, separate cash fee); Withdrawal (max 500,000 KRW, 3 per session, greedy dispensing); full REQ test plan, O/X checklist, screenshots, and demo checklist | 25% |
| Alan | Person D (Transfers, History, Admin, i18n, Exceptions & Report Assembly) | `src/Exceptions.h`, `src/Messages.*`, `src/CashTransferTransaction.*`, `src/AccountTransferTransaction.*`, `src/TransactionHistory.*`, `src/AdminMenu.*`, `tests/person_d_test.cpp`, `docs/final_report.md`, `docs/contribution_table.md`, `docs/demo_script.md` | ATMException hierarchy; English and Korean message tables; Cash Transfer and Account Transfer transactions; session-grouped TransactionHistory and file export; AdminMenu; Person D unit tests; final report assembly, contribution table, and demo rehearsal script | 25% |

Notes:
- Each member submits their own copy of the contribution table on Nov 29.
- Update the percentage column only if the team agrees on an adjustment before submission.
