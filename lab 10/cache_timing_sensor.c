#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(__rdtsc)
#pragma intrinsic(__rdtscp)
#elif defined(__GNUC__) || defined(__clang__)
#include <x86intrin.h>
#else
#error This lab requires an x86/x86-64 compiler with RDTSC support.
#endif

volatile uint64_t sink = 0;

enum {
    ARRAY_SIZE = 64 * 1024 * 1024,
    PAGE_STRIDE = 4096,
    EXPERIMENTS = 10000
};

typedef struct TimingStats {
    double average_cycles;
    double median_cycles;
    double average_ns;
    double median_ns;
} TimingStats;

static inline uint64_t read_tsc_begin(void) {
    _mm_lfence();
    return __rdtsc();
}

static inline uint64_t read_tsc_end(void) {
    unsigned int aux = 0;
    uint64_t cycles = __rdtscp(&aux);
    _mm_lfence();
    return cycles;
}

static int compare_uint64(const void* left, const void* right) {
    uint64_t left_value = *(const uint64_t*)left;
    uint64_t right_value = *(const uint64_t*)right;

    if (left_value < right_value) {
        return -1;
    }
    if (left_value > right_value) {
        return 1;
    }
    return 0;
}

static double average_cycles(const uint64_t* samples, int count) {
    double total = 0.0;
    int index = 0;

    for (index = 0; index < count; ++index) {
        total += (double)samples[index];
    }

    return total / (double)count;
}

static double median_cycles(const uint64_t* samples, int count) {
    uint64_t* copy = (uint64_t*)malloc((size_t)count * sizeof(uint64_t));
    int middle = count / 2;
    double result = 0.0;
    int index = 0;

    if (copy == NULL) {
        return -1.0;
    }

    for (index = 0; index < count; ++index) {
        copy[index] = samples[index];
    }

    qsort(copy, (size_t)count, sizeof(uint64_t), compare_uint64);

    if ((count % 2) == 0) {
        result = ((double)copy[middle - 1] + (double)copy[middle]) / 2.0;
    } else {
        result = (double)copy[middle];
    }

    free(copy);
    return result;
}

static double calibrate_cycles_per_second(void) {
#if defined(_WIN32)
    LARGE_INTEGER frequency;
    LARGE_INTEGER start_counter;
    LARGE_INTEGER end_counter;
    uint64_t start_tsc;
    uint64_t end_tsc;
    double elapsed_seconds;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start_counter);
    start_tsc = read_tsc_begin();
    Sleep(200);
    end_tsc = read_tsc_end();
    QueryPerformanceCounter(&end_counter);

    elapsed_seconds = (double)(end_counter.QuadPart - start_counter.QuadPart) / (double)frequency.QuadPart;
    return (double)(end_tsc - start_tsc) / elapsed_seconds;
#else
    return 0.0;
#endif
}

static void initialize_array(unsigned char* buffer, size_t length) {
    size_t offset = 0;
    for (offset = 0; offset < length; offset += PAGE_STRIDE) {
        buffer[offset] = (unsigned char)(offset & 0xFFu);
    }
}

static uint64_t measure_hit(unsigned char* address) {
    sink += *address;
    _mm_lfence();

    {
        uint64_t start = read_tsc_begin();
        sink += *address;
        return read_tsc_end() - start;
    }
}

static uint64_t measure_miss(unsigned char* address) {
    _mm_clflush(address);
    _mm_mfence();

    {
        uint64_t start = read_tsc_begin();
        sink += *address;
        return read_tsc_end() - start;
    }
}

static TimingStats build_stats(const uint64_t* samples, int count, double cycles_per_second) {
    TimingStats stats;
    const double nanoseconds_per_cycle = 1000000000.0 / cycles_per_second;

    stats.average_cycles = average_cycles(samples, count);
    stats.median_cycles = median_cycles(samples, count);
    stats.average_ns = stats.average_cycles * nanoseconds_per_cycle;
    stats.median_ns = stats.median_cycles * nanoseconds_per_cycle;
    return stats;
}

int main(void) {
    unsigned char* buffer = (unsigned char*)malloc(ARRAY_SIZE);
    uint64_t* hit_samples = (uint64_t*)malloc((size_t)EXPERIMENTS * sizeof(uint64_t));
    uint64_t* miss_samples = (uint64_t*)malloc((size_t)EXPERIMENTS * sizeof(uint64_t));
    unsigned char* hot_address;
    unsigned char* cold_address;
    double cycles_per_second;
    TimingStats hit_stats;
    TimingStats miss_stats;
    double hit_to_miss_ratio;
    double miss_to_hit_ratio;
    int experiment = 0;

    if (buffer == NULL || hit_samples == NULL || miss_samples == NULL) {
        fprintf(stderr, "Allocation failed.\n");
        free(buffer);
        free(hit_samples);
        free(miss_samples);
        return 1;
    }

    initialize_array(buffer, ARRAY_SIZE);
    hot_address = &buffer[64];
    cold_address = &buffer[ARRAY_SIZE - 64];

    cycles_per_second = calibrate_cycles_per_second();
    if (cycles_per_second <= 0.0) {
        fprintf(stderr, "Failed to calibrate the TSC frequency.\n");
        free(buffer);
        free(hit_samples);
        free(miss_samples);
        return 1;
    }

    for (experiment = 0; experiment < EXPERIMENTS; ++experiment) {
        hit_samples[experiment] = measure_hit(hot_address);
        miss_samples[experiment] = measure_miss(cold_address);
    }

    hit_stats = build_stats(hit_samples, EXPERIMENTS, cycles_per_second);
    miss_stats = build_stats(miss_samples, EXPERIMENTS, cycles_per_second);
    hit_to_miss_ratio = hit_stats.average_ns / miss_stats.average_ns;
    miss_to_hit_ratio = miss_stats.average_ns / hit_stats.average_ns;

    printf("CENG356 Lab 10 - Cache Timing Sensor\n");
    printf("Array size: %d bytes (64 MiB)\n", ARRAY_SIZE);
    printf("Experiments: %d\n\n", EXPERIMENTS);
    printf("Average cache hit time:  %.2f cycles (%.2f ns)\n", hit_stats.average_cycles, hit_stats.average_ns);
    printf("Average cache miss time: %.2f cycles (%.2f ns)\n", miss_stats.average_cycles, miss_stats.average_ns);
    printf("Hit/Miss ratio:          %.3f\n", hit_to_miss_ratio);
    printf("Miss/Hit slowdown:       %.2fx\n\n", miss_to_hit_ratio);
    printf("Median cache hit time:   %.2f cycles (%.2f ns)\n", hit_stats.median_cycles, hit_stats.median_ns);
    printf("Median cache miss time:  %.2f cycles (%.2f ns)\n", miss_stats.median_cycles, miss_stats.median_ns);
    printf("Deliverable: report the hit time, miss time, and their ratio from your machine.\n");

    free(buffer);
    free(hit_samples);
    free(miss_samples);
    return 0;
}