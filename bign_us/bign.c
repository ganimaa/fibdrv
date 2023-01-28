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
            cnt += __builtin_clz(src->nums[i]);
            return cnt;
        } else {
            cnt += 32;
        }
    }
    return cnt;
}

int bn_msb(const bn *src)
{
    if (!src)
        return 0;
    return sizeof(int) * 32 - bn_clz(src);
}

char *bn_tostring(const bn *src)
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
    // memmove(dest, p, strlen(p) + 1);
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
    // bn_resize(c, new_size);
    unsigned long carry = 0;
    for (unsigned int i = 0; i < new_size; i++) {
        unsigned int t1 = (i < a->size) ? a->nums[i] : 0;
        unsigned int t2 = (i < b->size) ? b->nums[i] : 0;
        carry = (unsigned long) t1 + t2 + carry;
        temp->nums[i] = carry;
        carry >>= 32;
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

/*
 * c = |a| - |b| and |a| - |b| > 0 must be true
 */
void bn_do_sub(const bn *a, const bn *b, bn *c)
{
    unsigned int new_size = MAX(a->size, b->size);
    // bn_resize(c, new_size);
    bn *temp = bn_init(new_size);
    long borrow = 0;
    for (int i = 0; i < new_size; i++) {
        unsigned int t1 = (i < a->size) ? a->nums[i] : 0;
        unsigned int t2 = (i < b->size) ? b->nums[i] : 0;
        borrow = (long) t1 - t2 + borrow;
        if (borrow < 0) {
            temp->nums[i] = borrow + (1L << 32);
            borrow = -1;
        } else {
            temp->nums[i] = borrow;
            borrow = 0;
        }
    }
    for (int i = new_size - 1; i >= 0; i--) {
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
    unsigned int new_size = bn_msb(a) + bn_msb(b);
    new_size = DIV_ROUND(new_size, 32) + !new_size;

    bn *temp = bn_init(new_size);
    for (int i = 0; i < a->size; i++) {
        for (int j = 0; j < b->size; j++) {
            unsigned long long int carry = 0;
            carry = (unsigned long long int) a->nums[i] * b->nums[i];
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
    // bn_resize(temp, new_size);
    temp->size = new_size + !new_size;
    bn_cpy(c, temp);
    bn_free(temp);
}

void bn_mul_add(bn *res, unsigned int offset, unsigned long long int carry)
{
    for (unsigned int i = offset; i < res->size; i++) {
        res->nums[i] = carry & 0xFFFFFFFF;
        carry >>= 32;
        if (!carry)
            return;
    }
}

void bn_lshift(bn *src, unsigned int shift)
{
    if (!src)
        return;
    int firstzero = bn_clz(src);
    shift = shift % 32;
    if (!shift)
        return;
    if (shift > firstzero)
        bn_resize(src, src->size + 1);
    for (int i = src->size - 1; i > 0; i--) {
        src->nums[i] = src->nums[i] << shift | src->nums[i - 1] >> (32 - shift);
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
    unsigned int *new_size = realloc(src->nums, sizeof(int) * size);
    if (!new_size)
        return;
    src->nums = new_size;
    if (size > src->size) {
        for (int i = src->size; i < size; i++) {
            src->nums[i] = 0;
        }
        // memset(src->nums + src->size, 0, sizeof(int) * (size - src->size));
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
    a->size = size | 1;
    a->sign = 0;
    a->nums = malloc(sizeof(int) * a->size);
    if (!a->nums) {
        free(a);
        return 0;
    }
    for (int i = 0; i < a->size; i++) {
        a->nums[i] = 0;
    }
    // memset(a->nums, 0, a->size * sizeof(int));
    return a;
}

void bn_cpy(bn *dest, const bn *src)
{
    if (dest->size != src->size)
        bn_resize(dest, src->size);
    dest->sign = src->sign;
    for (int i = 0; i < dest->size; i++) {
        dest->nums[i] = src->nums[i];
    }
    // memcpy(dest->nums, src->nums, sizeof(int) * src->size);
}