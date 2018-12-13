#ifndef BINT_H
#define BINT_H

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <ctype.h>
#include <cassert>
#include <math.h>

// Uncomment to disable assert()
// #define NDEBUG
#include <cassert>

constexpr int DIGITS = 9;                  // Decimal digits in each big integer array element.
constexpr uint64_t BASE = pow(10, DIGITS);
constexpr uint64_t LIMIT = BASE - 1;

class bint
{
public:
    bint ()
    : value(0), width(0)
    {
    }

    bint (size_t width)
    : width(width)
    {
        value = new uint64_t[width + 1];
        bzero(value, width * sizeof value[0]);
    }

    uint64_t parseDigits(const char* s, int len)
    {
        uint32_t num = 0;
        for (int i = 0; i < len; i++)
        {
            num = num * 10 + *s++ - '0';
        }
        return num;
    }

    bint (const char* s)
    {
        if (!s || ! *s)
        {
            width = 0;
            value = 0;
            return;
        }

        int d1 = strlen(s) / DIGITS;
        int d2 = strlen(s) % DIGITS;

        width = d1;
        if (d2) width++;

        value = new uint64_t[width];
        bzero(value, width * sizeof(uint64_t));

        int w = width - 1;
        if(d2)
        {
            value[w--] = parseDigits(s, d2);
            s = s + d2;
        }

        while (w >= 0)
        {
            value[w--] = parseDigits(s, DIGITS);
            s = s + DIGITS;
        }
    }

    bint (const bint& k) // copy constructor 
    {
        width = k.width;
        value = new uint64_t[k.width];
        memcpy(value, k.value, width * sizeof value[0]);
    }

    ~bint ()
    {
        delete[] value;
    }

    void operator= (const bint& k)
    {
        if (width != k.width)
        {
            width = k.width;
            delete[] value;    
            value = new uint64_t[k.width];
        }
        memcpy(value, k.value, width * sizeof value[0]);
    }

    void operator= (const char* s)
    {
        width = strlen(s);
        delete[] value;    

        value = new uint64_t[width];
        bzero(value, width * sizeof value[0]);

        int i = 0;
        const char* r = s + strlen(s) - 1;

        while (r >= s)
        {
            value[i] = *r - '0';
            i++;
            r--;
        }
    }

    const bint low(int mid) const
    {
        assert(width > 1);
        assert(mid < width);
    
        // Make a result half the size of this, containing low half of this
        bint low(mid);
        std::memcpy(low.value, value, mid * sizeof low.value[0]);
        return low; 
    }

    const bint high(int mid) const
    {
        assert(width > 1);
        assert(mid < width);

        // Make a result half the size of this, containing high half of this
        bint high(width - mid);
        std::memcpy(high.value, &value[mid], (width - mid) * sizeof high.value[0]);
        return high; 
    }

    bint shift (int n)
    {
        // Make a result of the required size
        bint result(this->width + n);
        memmove(&result.value[n], &value[0], width * sizeof value[0]);
        return result; 
    }

    bint operator+ (const bint& n)
    {
        // Ensure "a" operand is longer than "b" operand
        const bint *a = this;
        const bint *b = &n;
        if (n.width > width)
        {
            a = &n;
            b = this;
        }

        // Make a result of the same size as operand "a"
        bint sum(a->width);

        int i = 0;
        uint64_t s = 0;
        uint64_t carry = 0;
        uint64_t *aPtr = a->value;
        uint64_t *bPtr = b->value;
        uint64_t *sPtr = sum.value;
        while (i < b->width)
        {
            s = *aPtr++ + *bPtr++ + carry;
            carry = (s >= BASE);
            s -= BASE * carry;
            *sPtr++ = s;
            i++;
        }
        while (i < a->width)
        {
            s = *aPtr++ + carry;
            carry = (s >= BASE);
            s -= BASE * carry;
            *sPtr++ = s;
            i++;
        }
        sum.width += carry;
        *sPtr = carry;
        return sum; 
    }

    bint operator- (const bint& b)
    {
        // Demand this operand is wider than the a operand
        if (this->width < b.width)
        {
            std::cout << "!!!!! this: " << *this << " b: " << b << std::endl; 
        }
        assert(this->width >= b.width);

        // Make a result of the same size as this
        bint difference(this->width);

        uint64_t borrow = 0;
        int i = 0;
        uint64_t *aPtr = this->value;
        uint64_t *bPtr = b.value;
        uint64_t *dPtr = difference.value;
        while (i < b.width)
        {
            *aPtr -= borrow;
            borrow = (*aPtr < *bPtr);
            *dPtr = *aPtr + (BASE * borrow) - *bPtr;
            dPtr++;
            aPtr++;
            bPtr++;
            i++;
        }
        while (i < this->width)
        {
            *aPtr -= borrow;
            borrow = (*aPtr < 0);                           // FIXME: WTF? unsigned less than zero ?!
            *dPtr = *aPtr + (BASE * borrow);
            dPtr++;
            aPtr++;
            i++;
        }
        return difference;
    }

    bint simpleMul (uint64_t k) const
    {
        bint product(width);
        uint64_t carry = 0;
        int i = 0;
        for (i = 0; i < width; i++) {
            uint64_t p = value[i] * k + carry;
            if (p < BASE)
            {
                product.value[i] = p;
                carry = 0;
            }
            else
            {
                carry = p / BASE;
                product.value[i] = p % BASE;
            }
        }
        if (carry)
        {
            product.width++;
            product.value[i] = carry;
        }
        return product;
    }

    bint operator* (const bint& b)
    {
        // The base case(s), only one element in value, just do the multiply
        if (width == 1)
        {
            return b.simpleMul(value[0]);
        }
        if (b.width == 1)
        {
            return simpleMul(b.value[0]);
        }
        // Calculates the size of the numbers
        int m = (this->width);
        int m2 = m / 2;

        // Split the numbers in the middle
        bint high1 = this->high(m2);
        bint low1 = this->low(m2);
        bint high2 = b.high(m2);
        bint low2 = b.low(m2);

        // Do da karatsaba shuffle, yabba dabba do.
        bint z0 = low1 * low2;
        bint z1 = (low1 + high1) * (low2 + high2);
        bint z2 = high1 * high2;

        bint s1 = z1 - z2;
        bint s2 = s1 - z0;

        return  z2.shift(m2 * 2) + s2.shift(m2) + z0;
    }

    friend std::ostream& operator<<(std::ostream& os, const bint& b)
    {
        if (b.width == 0) 
        {
            os << "BINTNULL";
        }
        else
        {
            for (int i = b.width - 1; i >= 0; i--)
            {
                os << std::setfill('0') << std::setw(DIGITS) << b.value[i];
            }
        }
        return os;  
    }  

private:
    uint64_t* value;
    int32_t  width;
};

#endif // BINT_H
