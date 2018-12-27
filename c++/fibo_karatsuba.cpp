#include <time.h>
#include <unordered_map>
#include <time.h>

#include "bint.h"

const bint zero = "0";
const bint one = "1";
const bint two = "2";

int isEven(int n) { return (n & 1) == 0; }

std::unordered_map<uint64_t, bint> memo;

#if MUTABLE
bint fibo(int n) {
    if (memo.find(n) != memo.end()) {
        return memo[n];    
    }
    int k = (n / 2);
    bint fk = fibo(k);
    bint fk1 = fibo(k + 1);
    if (isEven(n)) {
        bint result = fk * (fk1 * two - fk); 
        memo[n] = result;
        return result;
    }
    bint result = (fk * fk) + (fk1 * fk1);
    memo[n] = result;
    return result; 
}
#else
const bint fiboEjolson (int n)
{
    switch (n)
    {
        case 0:
            return zero;
        case 1:
            return one;
        case 2:
            return one;
        default:
            int k = (n / 2);
            const bint fk = fibo(k);
            const bint fk1 = fibo(k + 1);
            if (isEven(n))
            {
                return fk * (fk1 * two - fk);
            }
            return (fk * fk) + (fk1 * fk1);
    }
}
#endif

// This version derived from Paeryn's Haskell example.
const bint fiboFast (int n) {
    if (memo.find(n) != memo.end()) {
        return memo[n];    
    }

    int k = (n / 2);
    const bint a = fiboFast(k);
    const bint b = fiboFast(k - 1);
    if (isEven(n)) {
        return memo[n] = a * (two * b + a);
    }
    if ((n % 4) == 1) {
        return memo[n] = (two * a + b) * (two * a - b) + two;
    }
    return memo[n] = (two * a + b) * (two * a - b) - two;
}

bint timeIt(int n) {
    double endTime;
    double elapsedTime;
    double startTime = clock() / CLOCKS_PER_SEC;

#if MUTABLE
    memo.clear();
    memo[0] = zero;
    memo[1] = one;
    memo[2] = one;
#endif
    bint res = fiboFast(n);

    endTime = (float)clock()/CLOCKS_PER_SEC;
    elapsedTime = endTime - startTime;
#if DEBUG
//    std::cout << "Compute time: " << elapsedTime << std::endl;
#endif
    return res;
}

int main(int argc, char *argv[]) {
    bint res = timeIt(4784969);
    std::cout << res << std::endl;

#if DEBUG
    std::cout << "allocWithWidth: " << allocWithWidth << std::endl;
    std::cout << "allocCopy: " << allocCopy << std::endl;
    std::cout << "allocString: " << allocString << std::endl;
    std::cout << "allocEquals: " << allocEquals << std::endl;
    std::cout << "allocEqualString: " << allocEqualsString << std::endl;
    std::cout << "allocHigh: " << allocHigh << std::endl;
    std::cout << "allocLow: " << allocLow << std::endl;
    std::cout << "allocShift: " << allocShift << std::endl;
    std::cout << "allocGrow: " << allocGrow << std::endl;
    std::cout << "allocBytes: " << allocBytes << std::endl;
    std::cout << "allocations: " << allocations << std::endl;
    std::cout << "mulCount: " << mulCount << std::endl;

    for (int i = 0; i < 17; i++) {
        std::cout << allocs[i] << std::endl;
    }
#endif
    return 0;
}
