#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <cx16.h>

// Test data
static volatile unsigned char test_char = 42;
static volatile unsigned int test_int = 12345;
static volatile unsigned long test_long = 123456789;
static unsigned char char_array[64];
static unsigned int int_array[64];
static unsigned long long_array[64];
static unsigned char char_array_2d[64][64];
static unsigned int int_array_2d[64][64];
static unsigned long long_array_2d[64][64];

// Benchmark variables
static clock_t start, end;
static unsigned long i;
static volatile unsigned char result_char = 0;
static volatile unsigned int result_int = 0;
static volatile unsigned long result_long = 0;
static volatile unsigned char char_temp = 10;
static volatile unsigned int int_temp = 10;
static volatile unsigned long long_temp = 10;

#define ITERATIONS 100000

void benchmark_reads(void) {
    printf("Benchmarking with %lu iterations\n\n", (unsigned long)ITERATIONS);
    printf("%-35s %12s\n", "Benchmark", "Clocks");
    printf("%-35s %12s\n", "----------", "------");
    
    // Initialize arrays
    for (i = 0; i < 64; i++) {
        char_array[i] = i;
        int_array[i] = i * 100;
        long_array[i] = i * 10000;
    }
    for (i = 0; i < 64; i++) {
        unsigned long j;
        for (j = 0; j < 64; j++) {
            char_array_2d[i][j] = i + j;
            int_array_2d[i][j] = (i + j) * 100;
            long_array_2d[i][j] = (i + j) * 10000;
        }
    }
    
    // Benchmark reading a char
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        result_char = test_char;
    }
    end = clock();
    printf("%-35s %12lu\n", "Read char (volatile)", (unsigned long)(end - start));
    
    // Benchmark reading an int
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        result_int = test_int;
    }
    end = clock();
    printf("%-35s %12lu\n", "Read int (volatile)", (unsigned long)(end - start));
    
    // Benchmark reading a long
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        result_long = test_long;
    }
    end = clock();
    printf("%-35s %12lu\n", "Read long (volatile)", (unsigned long)(end - start));
    
    // Benchmark reading last entry in 64-element char array
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        result_char = char_array[63];
    }
    end = clock();
    printf("%-35s %12lu\n", "Read char_array[63]", (unsigned long)(end - start));
    
    // Benchmark reading last entry in 64-element int array
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        result_int = int_array[63];
    }
    end = clock();
    printf("%-35s %12lu\n", "Read int_array[63]", (unsigned long)(end - start));
    
    // Benchmark reading last entry in 64-element long array
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        result_long = long_array[63];
    }
    end = clock();
    printf("%-35s %12lu\n", "Read long_array[63]", (unsigned long)(end - start));
    
    // Benchmark reading [63][63] from 64x64 char array
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        result_char = char_array_2d[63][63];
    }
    end = clock();
    printf("%-35s %12lu\n", "Read char_array_2d[63][63]", (unsigned long)(end - start));
    
    // Benchmark reading [63][63] from 64x64 int array
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        result_int = int_array_2d[63][63];
    }
    end = clock();
    printf("%-35s %12lu\n", "Read int_array_2d[63][63]", (unsigned long)(end - start));
    
    // Benchmark reading [63][63] from 64x64 long array
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        result_long = long_array_2d[63][63];
    }
    end = clock();
    printf("%-35s %12lu\n", "Read long_array_2d[63][63]", (unsigned long)(end - start));
    
    // Benchmark char arithmetic
    char_temp = 10;
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        char_temp = (char_temp + 5) * 2;
    }
    end = clock();
    printf("%-35s %12lu\n", "Char arithmetic", (unsigned long)(end - start));
    
    // Benchmark int arithmetic
    int_temp = 10;
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        int_temp = (int_temp + 5) * 2;
    }
    end = clock();
    printf("%-35s %12lu\n", "Int arithmetic", (unsigned long)(end - start));
    
    // Benchmark long arithmetic
    long_temp = 10;
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        long_temp = (long_temp + 5) * 2;
    }
    end = clock();
    printf("%-35s %12lu\n", "Long arithmetic", (unsigned long)(end - start));
}

int main(void) {
    asm("lda #1");
    asm("ldx #0");
    asm("ldy #0");
    asm("jsr $FF68");
    videomode(1);
    benchmark_reads();
    return 0;
}
