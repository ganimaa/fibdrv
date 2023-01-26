// #include <linux/slab.h>
// #include <linux/string.h>
// #include <linux/types.h>

#include "bign.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Count leading zeros of the bignum
// int bn_clz(bn *s)
// {
//     if (!s)
//         return 0;
//     int cnt = 0;
//     for (int i = s->size - 1; i >= 0; i--) {
//         if (s->nums[i]) {
//             cnt += __builtin_clz(s->nums[i]);
//             return cnt;
//         } else {
//             cnt += 32;
//         }
//     }
//     return cnt;
// }

// int bn_msb(bn *src)
// {
//     return sizeof(int) * 32 - bn_clz(src);
// }

char *bn_tostring(bn *src)
{
    // log10(x) = log2(x) / log2(10) ~= log2(x) / 3.322
    unsigned int len = src->size * 32;

    // Plus one is due to the end of the string '\0'
    len = DIV_ROUND(len, 3) + src->sign + 1;
    char *dest = malloc(len);
    char *p = dest;

    // Init the output string
    // memset(dest, '0', len - 1);
    for (int i = 0; i < len; i++) {
        dest[i] = '0';
    }
    dest[len - 1] = '\0';

    // Combine decimal string with the fast doubling logic
    for (int i = src->size - 1; i >= 0; i--) {
        for (unsigned int n = 1UL << 31; n; n >>= 1) {
            int carry = !!(n & src->nums[i]);
            for (int j = len - 2; j >= 0; j--) {
                dest[j] += dest[j] + carry - '0';
                carry = (dest[j] > '9');
                if (carry)
                    dest[j] -= 10;
            }
        }
    }
    // Skip the leading zeros
    while (p[0] == '0' && p[1] != '\0')
        p++;
    if (src->sign)
        *(--p) = '-';

    for (unsigned int i = 0; i < strlen(p) + 1; i++) {
        dest[i] = p[i];
    }
    return dest;
}

/*
 *  c = a + b
 */
void bn_add(bn *a, bn *b, bn *c)
{
    // Both of a and b are positive or negetive
    if (a->sign == b->sign) {
        bn_do_add(a, b, c);
        c->sign = a->sign;
    } else {
        // Keep parameter 'a' always positive
        if (a->sign)
            SWAP(a, b);
        // |a| = |b|
        if (bn_cmp(a, b) == 0) {
            c->nums[0] = 0;
            c->sign = 0;
        }
        // |a| > |b|, |b| < 0 => |c| = a - |b|
        if (bn_cmp(a, b) == 1) {
            bn_do_sub(a, b, c);
            c->sign = 0;
        }
        // |a| < |b|, |b| < 0 => |c| = -(|b| - |a|)
        if (bn_cmp(a, b) == -1) {
            bn_do_sub(b, a, c);
            c->sign = 1;
        }
    }
}

// void bn_sub(bn *a, bn *b, bn *c)
// {
//     bn *temp = b;
//     temp->sign = !temp->sign;
//     bn_add(a, b, c);
// }

static void bn_do_add(bn *a, bn *b, bn *c)
{
    unsigned int new_size = MAX(a->size, b->size) + 1;
    bn_resize(c, new_size);
    long carry = 0;
    for (unsigned int i = 0; i < c->size; i++) {
        unsigned int temp1 = (i < a->size) ? a->nums[i] : 0;
        unsigned int temp2 = (i < b->size) ? b->nums[i] : 0;
        carry = (long) temp1 + temp2 + carry;
        c->nums[i] = carry;
        carry >>= 32;
    }
}

/*
 * c = |a| - |b| and |a| - |b| > 0 must be true
 */
static void bn_do_sub(bn *a, bn *b, bn *c)
{
    bn_resize(c, MAX(a->size, b->size));
    long borrow = 0;
    for (int i = 0; i < c->size; i++) {
        unsigned int temp1 = (i < a->size) ? a->nums[i] : 0;
        unsigned int temp2 = (i < b->size) ? b->nums[i] : 0;
        borrow = (long) (temp1 - temp2) + borrow;
        if (borrow < 0) {
            c->nums[i] = borrow + (1L << 32);
            borrow = -1;
        } else {
            c->nums[i] = borrow;
            borrow = 0;
        }
    }
}

// void bn_mult(bn *a, bn *b, bn *c)
// {
//     unsigned int size = a->size + b->size;
//     bn_resize(c, size);
// }

int bn_cmp(bn *a, bn *b)
{
    if (a->size < b->size) {
        return -1;
    } else if (a->size > b->size) {
        return 1;
    } else {
        for (int i = a->size - 1; i >= 0; i--) {
            if (a->nums[i] > b->nums[i])
                return 1;
            if (a->nums[i] < b->nums[i])
                return -1;
        }
        return 0;
    }
}

// bn *bn_alloc(unsigned int size)
// {
//     bn *init = (bn *) malloc(sizeof(bn));
//     if (!init)
//         return 0;
//     init->nums = (unsigned int *) malloc(sizeof(int) * size);
//     if (!init->nums)
//         free(init);
//     for (unsigned int i = 0; i < size; i++) {
//         init->nums[i] = 0;
//     }
//     init->size = size;
//     init->sign = 0;
//     return init;
// }

int bn_resize(bn *src, int size)
{
    if (!src)
        return -1;
    if (src->size == size)
        return 0;
    if (size == 0)
        return bn_free(src);
    unsigned int *new_size = realloc(src->nums, sizeof(int) * size);
    if (!new_size)
        return -1;
    src->nums = new_size;
    if (size > src->size) {
        for (int i = src->size; i < size; i++) {
            src->nums[i] = 0;
        }
    }
    src->size = size;
    return 0;
}

int bn_free(bn *src)
{
    if (!src)
        return -1;
    free(src->nums);
    free(src);
    return 0;
}

bn *bn_init(void)
{
    bn *a = (bn *) malloc(sizeof(bn));
    if (!a)
        return NULL;
    a->size = 1;
    a->sign = 0;
    a->nums = malloc(sizeof(int) * a->size);
    if (!a->nums) {
        free(a);
        return 0;
    }
    for (int i = 0; i < a->size; i++) {
        a->nums[i] = 0;
    }
    return a;
}

bn *bn_fib_iter(unsigned int n)
{
    bn *res = bn_init();
    if (n < 2) {
        res->nums[0] = n & 1;
        return res;
    }
    // if (n == 1) {
    //     res->nums[0] = 1;
    //     return res;
    // }
    bn *a = bn_init();
    bn *b = bn_init();
    a->nums[0] = 0;
    b->nums[0] = 1;
    res->nums[0] = 1;
    for (int i = 2; i < n; i++) {
        bn_add(a, b, res);
        SWAP(a, res);
        bn_add(a, b, res);
        SWAP(b, a);
    }
    bn_free(a);
    bn_free(b);
    return res;
}