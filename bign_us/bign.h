
/*
 * Bignum structure
 * nums[n-1].....nums[0]
 *    MSB         LSB
 * size is the length of the nums array
 * sign = 1 means the number is negative
 */

typedef struct bignum {
    unsigned int *nums;
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
char *bn_tostring(bn *src);

/*
 * Find the first set of the bignum
 */
int bn_clz(bn *s);

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
void bn_add(bn *a, bn *b, bn *c);

/*
 * The logic of bn_sub() is as same as bn_add(),
 * reverse the sign of b and add it.
 */
void bn_sub(bn *a, bn *b, bn *c);


/*
 * c = |a| + |b|
 */
static void bn_do_add(bn *a, bn *b, bn *c);

/*
 * c = |a| - |b|
 */
static void bn_do_sub(bn *a, bn *b, bn *c);

/*
 * Return the MSB of the bignum
 */
int bn_msb(bn *src);


/*
 * Compare the bignum a and b which is larger,
 * if |a| > |b|, return 1
 * if |a| = |b|, return 0
 * if |a| < |b|, return -1
 */
int bn_cmp(bn *a, bn *b);

/*
 * Alloc a bn structure with the given size
 * and do initialized.
 */
bn *bn_alloc(unsigned int size);

/*
 * Replace the bn->size with the input size and
 * realloc the bn->nums, return 0 means success,
 * -1 means faild.
 * Ignore the data loss when the size changing.
 */
int bn_resize(bn *src, int size);

/*
 * Free entire bn data structure
 * return 0 to success, -1 to faild
 */
int bn_free(bn *src);
int bn_mul(bn *a, bn *b, bn *c);

/*
 * Calculate the n-th fibonacci sequence
 * by iterative.
 */
bn *bn_fib_iter(unsigned n);

bn *bn_init(void);