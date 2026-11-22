// Owner: Alan (Person D)
#pragma once
#include <stdexcept>
#include <string>

// Base exception class; caught in Session to end the current session (REQ9.1)
class ATMException : public std::runtime_error {
public:
    ATMException(const std::string& message) : std::runtime_error(message) {}
};


class InvalidCardException : public ATMException {
public:
    InvalidCardException(const std::string& message = "Invalid card.")
        : ATMException(message) {}
};

class WrongPasswordException : public ATMException {
public:
    WrongPasswordException(const std::string& message = "Incorrect password.")
        : ATMException(message) {}
};

class InsufficientFundsException : public ATMException {
public:
    InsufficientFundsException(const std::string& message = "Insufficient funds in account.")
        : ATMException(message) {}
};

class InsufficientCashException : public ATMException {
public:
    InsufficientCashException(const std::string& message = "Insufficient cash in ATM.")
        : ATMException(message) {}
};

class LimitExceededException : public ATMException {
public:
    LimitExceededException(const std::string& message = "Transaction limit exceeded.")
        : ATMException(message) {}
};

class InvalidInputException : public ATMException {
public:
    InvalidInputException(const std::string& message = "Invalid input.")
        : ATMException(message) {}
};
