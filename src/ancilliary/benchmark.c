#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

// Alternative PRNG implementations
// Simple Linear Congruential Generator (LCG)
static uint32_t lcg_state = 1;

uint32_t lcg_next(void) {
    lcg_state = (lcg_state * 1103515245 + 12345) & 0x7fffffff;
    return lcg_state;
}

void lcg_seed(uint32_t seed) {
    lcg_state = seed;
}

// Xorshift32 - Fast and decent quality
static uint32_t xorshift_state = 1;

uint32_t xorshift32_next(void) {
    xorshift_state ^= xorshift_state << 13;
    xorshift_state ^= xorshift_state >> 17;
    xorshift_state ^= xorshift_state << 5;
    return xorshift_state;
}

void xorshift32_seed(uint32_t seed) {
    xorshift_state = seed;
}

// Xorshift8 - Ultra-fast, minimal state
static uint8_t xorshift8_state = 1;

uint8_t xorshift8_next(void) {
    xorshift8_state ^= xorshift8_state << 7;
    xorshift8_state ^= xorshift8_state >> 5;
    xorshift8_state ^= xorshift8_state << 3;
    return xorshift8_state;
}

void xorshift8_seed(uint8_t seed) {
    xorshift8_state = seed;
}

// Simple feedback shift register (LFSR) - very fast
static uint16_t lfsr_state = 0xACE1;

uint16_t lfsr_next(void) {
    uint16_t bit = ((lfsr_state >> 0) ^ (lfsr_state >> 2) ^ (lfsr_state >> 3) ^ (lfsr_state >> 5)) & 1;
    lfsr_state = (lfsr_state >> 1) | (bit << 15);
    return lfsr_state;
}

void lfsr_seed(uint16_t seed) {
    lfsr_state = seed;
}

#define ITERATIONS 100000

void benchmark_rand(void) {
    clock_t start, end;
    unsigned long i;
    volatile unsigned int result = 0;  // volatile to prevent optimization
    
    printf("Benchmarking with %lu iterations\n\n", (unsigned long)ITERATIONS);
    
    // Benchmark stdlib rand()
    printf("stdlib rand():\n");
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        result = rand();
    }
    end = clock();
    printf("  Time: %lu clocks\n\n", (unsigned long)(end - start));
    
    // Benchmark LCG
    printf("Linear Congruential Generator (LCG):\n");
    lcg_seed(time(NULL));
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        result = lcg_next();
    }
    end = clock();
    printf("  Time: %lu clocks\n\n", (unsigned long)(end - start));
    
    // Benchmark Xorshift32
    printf("Xorshift32:\n");
    xorshift32_seed(time(NULL));
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        result = xorshift32_next();
    }
    end = clock();
    printf("  Time: %lu clocks\n\n", (unsigned long)(end - start));
    
    // Benchmark Xorshift8
    printf("Xorshift8 (8-bit):\n");
    xorshift8_seed(time(NULL));
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        result = xorshift8_next();
    }
    end = clock();
    printf("  Time: %lu clocks\n\n", (unsigned long)(end - start));
    
    // Benchmark LFSR
    printf("Linear Feedback Shift Register (LFSR):\n");
    lfsr_seed(time(NULL));
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        result = lfsr_next();
    }
    end = clock();
    printf("  Time: %lu clocks\n\n", (unsigned long)(end - start));
}

int main(void) {
    printf("\n=== Random Number Generator Benchmark ===\n");
    srand(time(NULL));
    benchmark_rand();
    printf("=== Benchmark Complete ===\n\n");
    return 0;
}
