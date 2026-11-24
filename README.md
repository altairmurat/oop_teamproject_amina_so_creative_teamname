# oop_teamproject
Rabia, Amina, Alan, Altair

CSE201 ATM term project: a console C++ program that simulates banks and ATMs.

## Build and run

Requires a C++17 compiler (g++ 7+, clang 5+ or Visual Studio 2019+).
Always run the program from the project folder, so `data/initial_conditions.txt` is found.

### g++ / MinGW / Linux / macOS

```
make                 # builds ./atm
make run             # ./atm data/initial_conditions.txt
make test-bank       # bank-side tests (Card, Account, Bank, SystemLoader)
```

Without make:

```
g++ -std=c++17 -O2 -o atm src/*.cpp
./atm data/initial_conditions.txt
```

### CMake

```
mkdir build && cd build && cmake .. && cmake --build .
./atm ../data/initial_conditions.txt
```

### Visual Studio (Windows)

Open "x64 Native Tools Command Prompt for VS", go to the project folder and run:

```
mkdir build
cl /std:c++17 /EHsc /utf-8 /Fobuild\ /Fe:build\atm.exe src\*.cpp
build\atm.exe data\initial_conditions.txt
```

`/utf-8` is needed because the Korean messages are stored as UTF-8. If Korean text
looks broken in the console, run `chcp 65001` first.

## How to use

1. The program loads every bank, account and ATM from the file given as the first
   argument (default `data/initial_conditions.txt`). Nothing is hard-coded.
2. Pick an ATM from the list (`0` exits).
3. Enter a card number (for example `1111-1111-1111` for Chris, or the admin card `0000`),
   then the password.
4. Type `/` at any prompt to print the snapshot of all ATM cash and all account balances.

The program has to be restarted before every test case so the initial conditions are the same.

## Initial condition file format

```
B A N                      number of banks, accounts, ATMs
<bank name>                x B
<bank>                     \
<user name>                 |
<account number>            |  x A   (12 digits, dashes allowed)
<funds>                     |        ("4000" or "4,000")
<card number>               |        (unique, one card = one account)
<password>                 /
<primary bank>             \
<serial number>             |  x N   (6 digits, unique)
Single | Multi              |
Unilingual | Bilingual      |
<50k> <10k> <5k> <1k>      /         (bill counts)
Admin <card number>        optional, any number of lines
```

Blank lines and lines starting with `#` are ignored. If no `Admin` line is
given, card number `0000` is the admin card. A broken file stops the program
with a message that says which line is wrong.
