// Owner: Amina
// every fee rule from REQ1.8 lives here

class FeePolicy {
public:
    // primary = the account's bank is the ATM's primary bank
    static int depositFee(bool primary); // paid in extra cash
    static int withdrawalFee(bool primary); // paid from the account
    static int accountTransferFee(bool srcPrimary, bool dstPrimary); // paid from the source account
    static int cashTransferFee(); // paid in extra cash
};