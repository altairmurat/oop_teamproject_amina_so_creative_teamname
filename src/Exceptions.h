// TEMPORARY version so Person C's code compiles. Owner: Person D -- replace/extend with the real one.
#pragma once
#include <stdexcept>
#include <string>

class ATMException : public std::runtime_error {
public:
    explicit ATMException(const std::string& msg) : std::runtime_error(msg) {}
};
class InvalidInputException : public ATMException {
public:
    explicit InvalidInputException(const std::string& msg) : ATMException(msg) {}
};
class InsufficientFundsException : public ATMException {
public:
    explicit InsufficientFundsException(const std::string& msg) : ATMException(msg) {}
};
class InsufficientCashException : public ATMException {
public:
    explicit InsufficientCashException(const std::string& msg) : ATMException(msg) {}
};
class LimitExceededException : public ATMException {
public:
    explicit LimitExceededException(const std::string& msg) : ATMException(msg) {}
};
