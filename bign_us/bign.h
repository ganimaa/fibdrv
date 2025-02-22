#include <stdint.h>

#if defined(__LP64__) || defined(__x86_64__) || defined(__amd64__) || \
    defined(__aarch64__)
#define BN_WSIZE 8
#else
#define BN_WSIZE 4
#endif

#if BN_WSIZE == 8
typedef uint64_t bign;
typedef unsigned __int128 bign_t;  // gcc support __int128
#define BITS 64U
#define DATA_MASK __UINT64_C(0xffffffffffffffff)
#define CLZ(x) __builtin_clzll(x)
#elif BN_WSIZE == 4
typedef uint32_t bign;
typedef uint64_t bign_t;
#define BITS 32U
#define DATA_MASK __UINT32_C(0xffffffff)
#define CLZ(x) __builtin_clz(x)
#else
#error "BN_WSIZE must be 4 or 8"
#endif

#define digit_mul(u, v, hi, lo) \
    __asm__("mulq %3" : "=a"(lo), "=d"(hi) : "%0"(u), "rm"(v))

/*
 * Bignum structure
 * nums[n-1].....nums[0]
 *    MSB         LSB
 * size is the length of the nums array
 * sign = 1 means the number is negative
 */

typedef struct bignum {
    bign *nums;
    unsigned int size;
    int sign;
} bn;

#define SWAP(a, b)            \
    ({                        \
        do {                  \
            typeof(a) _t = a; \
            a = b;            \
            b = _t;           \
        } while (0);          \
    })

#define MAX(a, b)          \
    ({                     \
        typeof(a) _a = a;  \
        typeof(b) _b = b;  \
        _a > _b ? _a : _b; \
    })

#ifndef DIV_ROUND
#define DIV_ROUND(x, len) (((x) + (len) -1) / (len))
#endif

/*
 * out put the bignum to a string
 */
char *bn_tostring(const bn *src);

/*
 * Find the first set of the bignum
 */
int bn_clz(const bn *s);

/*
 * bn_add() is a function which calculate "c =  a + b",
 * this function consider the situation below:
 * 1. a and b are both postive or negative
 * 2. |a| > |b| and b < 0, then c = a - |b|
 * 3. |a| < |b| and b < 0, then c = -(|b| - |a|)
 * 4. |a| = |b| then c = 0
 * To simplify the computational logic, this function
 * assume that "a > 0", if "a > 0" , implement
 * SWAP(a, b) to ensure "a > 0"
 */
void bn_add(const bn *a, const bn *b, bn *c);

/*
 * The logic of bn_sub() is as same as bn_add(),
 * reverse the sign of b and add it.
 */
void bn_sub(const bn *a, const bn *b, bn *c);

/*
 * c = |a| + |b|
 */
void bn_do_add(const bn *a, const bn *b, bn *c);

/*
 * c = |a| - |b|
 */
void bn_do_sub(const bn *a, const bn *b, bn *c);

/*
 * Compare the bignum a and b which is larger,
 * if |a| > |b|, return 1
 * if |a| = |b|, return 0
 * if |a| < |b|, return -1
 */
int bn_cmp(const bn *a, const bn *b);

/*
 * Replace the bn->size with the input size and
 * realloc the bn->nums, return 0 means success,
 * -1 means faild.
 * Ignore the data loss when the size changing.
 */
void bn_resize(bn *src, unsigned int size);

/*
 * Free entire bn data structure
 * return 0 to success, -1 to faild
 */
int bn_free(bn *src);

/*
 * To initialize the bn structure with,
 * sign    = 0,
 * size    = 1,
 * nums[0] = 0
 */
bn *bn_init(unsigned int size);

/*
 * c = a * b
 */
void bn_mul(const bn *a, const bn *b, bn *c);

/*
 * Calculate c[i + j] = a[i] + b[j]
 */
bign mul_add_v2(const bn *a, const bign k, bign *c);

/*
 * Copy a bn structure to another bn structure
 */
void bn_cpy(bn *dest, const bn *src);

/*
 * Left shift for the bn structure,
 * shift 31 digits at most.
 */
void bn_lshift(bn *src, unsigned int shift);

/*
 * Calculate the fibonacci sequence with
 * iterate method.
 */
bn *bn_fib_iter(unsigned int n);

/*
 * Calculate the fibonacci sequence with
 * fast doubling method.
 */
bn *bn_fib_fast(unsigned int n);