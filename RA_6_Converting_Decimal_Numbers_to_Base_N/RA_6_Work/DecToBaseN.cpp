
#include <string>
#include <iostream>
using namespace std;

static char digitOf(int v) {
    const char DIGITS[] = "0123456789ABCDEF";
    return DIGITS[v];
}


static string to_base_rec(unsigned int x, int base) {
    if (x < static_cast<unsigned int>(base)) {
        return string(1, digitOf(static_cast<int>(x)));
    }
   
    return to_base_rec(x / static_cast<unsigned int>(base), base)
         + digitOf(static_cast<int>(x % static_cast<unsigned int>(base)));
}

string DecToBaseN(int num, int base) {
    
    if (base < 2 || base > 16) {
        cout << "Invalid Base: " << base << endl;
        return "";
    }

    
    if (num == 0) return "0";

    
    bool neg = (num < 0);
    unsigned int u = neg ? static_cast<unsigned int>(-(long long)num)
                         : static_cast<unsigned int>(num);

    string out = to_base_rec(u, base);
    if (neg) out.insert(out.begin(), '-');
    return out;
}
