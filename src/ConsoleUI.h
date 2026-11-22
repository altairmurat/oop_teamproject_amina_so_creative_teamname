// Owner: Altair (Person B)
#pragma once
#include <functional>
#include <iostream>
#include <string>
#include "Constants.h"

class ATM;

// this is to handle all console input/output safely and catch '/' for snapshots
class ConsoleUI {
public:
    static void setSnapshotCallback(std::function<void()> callback);
    static void triggerSnapshot();

    // reads a line and intercepts '/' anywhere to print the snapshot
    static std::string readLine(const std::string& prompt = "");

    static int readMenuChoice(const std::string& prompt, int minVal, int maxVal);
    static long long readPositiveAmount(const std::string& prompt);
    static int readInt(const std::string& prompt, int minVal, int maxVal);
    static long long readLongLong(const std::string& prompt, long long minVal, long long maxVal);
    static BillCount readBills(int maxTotalBills = Constants::MAX_BILLS_PER_DEPOSIT);

    static Language promptLanguage(const ATM& atm);
    static void showMessage(const std::string& msg);

private:
    static std::function<void()> snapshotCallback;
    static std::string trim(const std::string& s);
};
