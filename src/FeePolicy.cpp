// Owner: Amina
#include "FeePolicy.h"

int FeePolicy::depositFee(bool primary) { return primary ? 0 : 2000; }

int FeePolicy::withdrawalFee(bool primary) { return primary ? 2000 : 3000; }

int FeePolicy::accountTransferFee(bool srcPrimary, bool dstPrimary) {
    if (srcPrimary && dstPrimary) return 2000; // primary -> primary
    if (!srcPrimary && !dstPrimary) return 5000; // non-primary -> non-primary
    return 3000; // only one is primary
}

int FeePolicy::cashTransferFee() { return 1000; }