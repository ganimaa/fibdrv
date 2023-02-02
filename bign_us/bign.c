// #include <linux/slab.h>
// #include <linux/string.h>
// #include <linux/types.h>

#include "bign.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Count leading zeros of the bignum
int bn_clz(const bn *src)
{
    if (!src)
        return 0;
    int cnt = 0;
    for (int i = src->size - 1; i >= 0; i--) {
        if (src->nums[i]) {
            cnt += CLZ(src->nums[i]);
            return cnt;
        } else {
            cnt += BITS;
        }
    }
    return cnt;
}

char *bn_tostring(const bn *src)
{
    // log10(x) = log2(x) / log2(10) ~= log2(x) / 3.BITS2
    unsigned int len = src->size * BITS;

    // Plus one is due to the end of the string '\0'
    len = DIV_ROUND(len, 3) + src->sign + 1;
    char *dest = malloc(len);
    char *p = dest;

    // Init the output string
    memset(dest, '0', len - 1);
    dest[len - 1] = '\0';

    // Combine decimal string with the fast doubling logic
    for (int i = src->size - 1; i >= 0; i--) {
        for (bign n = 1UL << (BITS - 1); n; n >>= 1) {
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
    memmove(dest, p, strlen(p) + 1);
    return dest;
}

/*
 *  c = a + b
 */
void bn_add(const bn *a, const bn *b, bn *c)
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
            bn_resize(c, 1);
            c->nums[0] = 0;
            c->sign = 0;
        }
        // |a| > |b|, b < 0 => c = a - |b|
        if (bn_cmp(a, b) == 1) {
            bn_do_sub(a, b, c);
            c->sign = 0;
        }
        // |a| < |b|, b < 0 => c = -(|b| - |a|)
        if (bn_cmp(a, b) == -1) {
            bn_do_sub(b, a, c);
            c->sign = 1;
        }
    }
}

void bn_sub(const bn *a, const bn *b, bn *c)
{
    bn *temp = bn_init(b->size);
    bn_cpy(temp, b);
    temp->sign = !temp->sign;
    bn_add(a, temp, c);
    bn_free(temp);
}

void bn_do_add(const bn *a, const bn *b, bn *c)
{
    unsigned int new_size = MAX(a->size, b->size) + 1;
    bn *temp = bn_init(new_size);
    bign_t carry = 0;
    for (unsigned int i = 0; i < new_size; i++) {
        bign t1 = (i < a->size) ? a->nums[i] : 0;
        bign t2 = (i < b->size) ? b->nums[i] : 0;
        carry = (bign_t) t1 + t2 + carry;
        temp->nums[i] = carry & DATA_MASK;
        carry >>= BITS;
    }
    for (int i = new_size - 1; i > 0; i--) {
        if (!temp->nums[i])
            new_size--;
        else
            break;
    }
    temp->size = new_size;
    bn_cpy(c, temp);
    bn_free(temp);
}

/*
 * c = |a| - |b| and |a| - |b| > 0 must be true
 */
void bn_do_sub(const bn *a, const bn *b, bn *c)
{
    unsigned int new_size = MAX(a->size, b->size);
    bn *temp = bn_init(new_size);
    bign_t borrow = 0;
    for (int i = 0; i < new_size; i++) {
        bign_t t1 = (i < a->size) ? a->nums[i] : 0;
        bign_t t2 = (i < b->size) ? b->nums[i] : 0;
        t1 += DATA_MASK + 1;
        t2 += borrow;
        borrow = t1 - t2;
        temp->nums[i] = borrow & DATA_MASK;
        borrow = (borrow > DATA_MASK);
    }
    for (int i = new_size - 1; i > 0; i--) {
        if (!temp->nums[i])
            new_size--;
        else
            break;
    }
    temp->size = new_size + !new_size;
    bn_cpy(c, temp);
    bn_free(temp);
}

void bn_mul(const bn *a, const bn *b, bn *c)
{
    unsigned int new_size = a->size + b->size + 1;

    bn *temp = bn_init(new_size);
    for (int i = 0; i < a->size; i++) {
        for (int j = 0; j < b->size; j++) {
            bign_t carry = 0;
            carry = (bign_t) a->nums[i] * b->nums[j];
            bn_mul_add(temp, i + j, carry);
        }
    }
    for (int i = new_size - 1; i > 0; i--) {
        if (!temp->nums[i])
            new_size--;
        else
            break;
    }
    temp->sign = a->sign ^ b->sign;
    temp->size = new_size + !new_size;
    bn_cpy(c, temp);
    bn_free(temp);
}

void bn_mul_add(bn *res, unsigned int offset, bign_t carry)
{
    bign_t x = 0;
    for (unsigned int i = offset; i < res->size; i++) {
        x += res->nums[i] + (carry & DATA_MASK);
        res->nums[i] = x;
        carry >>= BITS;
        x >>= BITS;
        if (!carry && !x)
            return;
    }
}

void bn_lshift(bn *src, unsigned int shift)
{
    if (!src)
        return;
    int firstzero = bn_clz(src);
    shift = shift % BITS;
    if (!shift)
        return;
    if (shift > firstzero)
        bn_resize(src, src->size + 1);
    for (int i = src->size - 1; i > 0; i--) {
        src->nums[i] =
            src->nums[i] << shift | src->nums[i - 1] >> (BITS - shift);
    }
    src->nums[0] <<= shift;
}

int bn_cmp(const bn *a, const bn *b)
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

void bn_resize(bn *src, unsigned int size)
{
    if (!src || src->size == size)
        return;
    if (!size)
        size = 1;
    bign *new_nums = realloc(src->nums, sizeof(bign) * size);
    if (!new_nums)
        return;
    src->nums = new_nums;
    if (size > src->size) {
        memset(src->nums + src->size, 0, sizeof(bign) * (size - src->size));
    }
    src->size = size;
}

int bn_free(bn *src)
{
    if (!src)
        return -1;
    free(src->nums);
    free(src);
    return 0;
}

bn *bn_init(unsigned int size)
{
    bn *a = (bn *) malloc(sizeof(bn));
    if (!a)
        return NULL;
    a->size = size + !size;
    a->sign = 0;
    a->nums = malloc(sizeof(bign) * a->size);
    if (!a->nums) {
        free(a);
        return 0;
    }
    memset(a->nums, 0, a->size * sizeof(bign));
    return a;
}

void bn_cpy(bn *dest, const bn *src)
{
    if (dest->size != src->size)
        bn_resize(dest, src->size);
    dest->sign = src->sign;
    memcpy(dest->nums, src->nums, sizeof(bign) * src->size);
}

bn *bn_fib_iter(unsigned int n)
{
    bn *res = bn_init(1);
    if (n < 2) {
        res->nums[0] = n;
        return res;
    }

    bn *a, *b;
    a = bn_init(1);
    b = bn_init(1);
    a->nums[0] = 0;
    b->nums[0] = 1;
    res->nums[0] = 1;
    for (int i = 2; i <= n; i++) {
        bn_add(res, b, res);
        SWAP(res, b);
    }
    bn_free(a);
    bn_free(b);
    return res;
}

bn *bn_fib_fast(unsigned int n)
{
    bn *fib0 = bn_init(1);
    if (n < 2) {
        fib0->nums[0] = n;
        return fib0;
    }
    bn *fib1 = bn_init(1);
    fib0->nums[0] = 0;
    fib1->nums[0] = 1;
    bn *k1 = bn_init(1);
    bn *k2 = bn_init(1);
    int k = BITS - CLZ(n);
    for (bign i = 1U << (k - 1); i; i >>= 1) {
        // F(2k) = F(k) * [ 2 * F(k+1) – F(k) ]
        bn_cpy(k1, fib1);
        bn_lshift(k1, 1);
        bn_sub(k1, fib0, k1);
        bn_mul(k1, fib0, k1);
        // F(2k+1) = F(k)^2 + F(k+1)^2
        bn_mul(fib0, fib0, fib0);
        bn_mul(fib1, fib1, fib1);
        bn_add(fib0, fib1, k2);
        SWAP(fib0, k1);
        SWAP(fib1, k2);
        if (n & i) {
            SWAP(fib0, fib1);
            bn_add(fib0, fib1, fib1);
        }
    }
    bn_free(fib1);
    bn_free(k1);
    bn_free(k2);
    return fib0;
}