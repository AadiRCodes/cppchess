#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <time.h>
typedef unsigned long long U64;

U64 seed = 1804289383;
U64 xorShift() {
    U64 x = seed;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    seed = x;
    return x;
}
U64 generate_random_U64() {
    U64 u1, u2, u3, u4;
    u1 = (U64)(xorShift()) & 0xFFFF; 
    u2 = (U64)(xorShift()) & 0xFFFF;
    u3 = (U64)(xorShift()) & 0xFFFF; 
    u4 = (U64)(xorShift()) & 0xFFFF;
    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}
// Generate random 64 bit number with "few" bits.
U64 magic_candidate() {
    return generate_random_U64() & generate_random_U64() & generate_random_U64();
}

