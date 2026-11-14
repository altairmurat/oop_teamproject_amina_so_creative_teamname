#pragma once

#include <exception>
#include <string>

// ребята используйте эти классы как основу для ошибок, названия пж не меняйте
// base exception class, is catched in Session and session will be ended (REQ9.1)
class ATMException : public std::exception {
public:
    explicit ATMException(const std::string& message) : msg(message) {}

    const char* what() const noexcept override {
        return msg.c_str();
    }

private:
    std::string msg;
};

class InvalidCardException : public ATMException {
public:
    explicit InvalidCardException(const std::string& message = "Invalid card.")
        : ATMException(message) {}
};

class WrongPasswordException : public ATMException {
public:
    explicit WrongPasswordException(const std::string& message = "Incorrect password.")
        : ATMException(message) {}
};

class InsufficientFundsException : public ATMException {
public:
    explicit InsufficientFundsException(const std::string& message = "Insufficient funds in account.")
        : ATMException(message) {}
};

class InsufficientCashException : public ATMException {
public:
    explicit InsufficientCashException(const std::string& message = "Insufficient cash in ATM.")
        : ATMException(message) {}
};

class LimitExceededException : public ATMException {
public:
    explicit LimitExceededException(const std::string& message = "Transaction limit exceeded.")
        : ATMException(message) {}
};

class InvalidInputException : public ATMException {
public:
    explicit InvalidInputException(const std::string& message = "Invalid input.")
        : ATMException(message) {}
};
