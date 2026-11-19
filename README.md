# oop_teamproject
Rabia, Amina, Alan, Altair

CSE201 ATM term project: a console C++ program that simulates banks and ATMs.

## Build and run

Requires a C++17 compiler (g++ 7+ or clang 5+).

```
make                 # builds ./atm
make run             # ./atm data/initial_conditions.txt
make test-bank       # bank-side tests (Card, Account, Bank, SystemLoader)
```

Or with CMake:

```
mkdir build && cd build && cmake .. && cmake --build .
./atm ../data/initial_conditions.txt
```

The program loads every bank, account and ATM from the file given as the first
argument (default `data/initial_conditions.txt`). Nothing is hard-coded.

## Initial condition file format

```
B A N                      number of banks, accounts, ATMs
<bank name>                x B
<bank>                     \
<user name>                 |
<account number>            |  x A   (12 digits, dashes allowed)
<funds>                     |
<card number>               |
<password>                 /
<primary bank>             \
<serial number>             |  x N   (6 digits)
Single | Multi              |
Unilingual | Bilingual      |
<50k> <10k> <5k> <1k>      /         (bill counts)
Admin <card number>        optional, any number of lines
```

Blank lines and lines starting with `#` are ignored. If no `Admin` line is
given, card number `0000` is the admin card.
