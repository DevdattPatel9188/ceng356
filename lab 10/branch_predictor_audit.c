#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE __attribute__((noinline))
#endif

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

NOINLINE void hot_if_path(void) {
    sink += 1;
}

NOINLINE void cold_else_path(void) {
    sink += 7;
}

NOINLINE void branch_probe(bool take_if_path) {
    if (take_if_path) {
        hot_if_path();
    } else {
        cold_else_path();
    }
}

NOINLINE void run_training_window(bool mismatch_on_last_branch, int repeated_windows) {
    int window = 0;
    for (window = 0; window < repeated_windows; ++window) {
        int iteration = 0;
        for (iteration = 0; iteration < 99; ++iteration) {
            branch_probe(true);
        }
        branch_probe(!mismatch_on_last_branch);
    }
}

uint64_t measure_window(bool mismatch_on_last_branch, int repeated_windows) {
    uint64_t start = read_tsc_begin();
    run_training_window(mismatch_on_last_branch, repeated_windows);
    return read_tsc_end() - start;
}

double average_cycles(const uint64_t* samples, int count) {
    double total = 0.0;
    int index = 0;
    for (index = 0; index < count; ++index) {
        total += (double)samples[index];
    }
    return total / (double)count;
}

int compare_uint64(const void* left, const void* right) {
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

double median_cycles(const uint64_t* samples, int count) {
    uint64_t* copy = (uint64_t*)malloc((size_t)count * sizeof(uint64_t));
    int middle = count / 2;
    double result = 0.0;

    if (copy == NULL) {
        return -1.0;
    }

    for (int index = 0; index < count; ++index) {
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

int main(void) {
    enum { experiments = 4000, repeated_windows = 256 };
    uint64_t trained_samples[experiments];
    uint64_t mismatched_samples[experiments];
    int experiment = 0;

    for (experiment = 0; experiment < experiments; ++experiment) {
        trained_samples[experiment] = measure_window(false, repeated_windows);
        mismatched_samples[experiment] = measure_window(true, repeated_windows);
    }

    {
        double trained_average = average_cycles(trained_samples, experiments);
        double mismatched_average = average_cycles(mismatched_samples, experiments);
        double trained_median = median_cycles(trained_samples, experiments);
        double mismatched_median = median_cycles(mismatched_samples, experiments);
        double penalty = (mismatched_average - trained_average) / (double)repeated_windows;
        double median_penalty = (mismatched_median - trained_median) / (double)repeated_windows;
        double trained_window_average = trained_average / (double)repeated_windows;
        double mismatched_window_average = mismatched_average / (double)repeated_windows;
        double trained_window_median = trained_median / (double)repeated_windows;
        double mismatched_window_median = mismatched_median / (double)repeated_windows;

        printf("CENG356 Lab 10 - Microarchitectural Side-Channel Audit\n");
        printf("Experiments: %d\n\n", experiments);
        printf("Each timed sample repeats the 99x if-training pattern %d times and then normalizes back to cycles per 100-iteration window.\n\n", repeated_windows);
        printf("Average cycles for trained if branch window:      %.2f\n", trained_window_average);
        printf("Average cycles for mismatched else branch window: %.2f\n", mismatched_window_average);
        printf("Observed mismatch penalty:                 %.2f cycles\n\n", penalty);
        printf("Median cycles for trained if branch window:       %.2f\n", trained_window_median);
        printf("Median cycles for mismatched else branch window:  %.2f\n", mismatched_window_median);
        printf("Median mismatch penalty:                   %.2f cycles\n\n", median_penalty);
        printf("Expect run-to-run noise. The mismatch result is the key signal because it breaks the predictor's training pattern on the 100th branch.\n");
    }

    return 0;
}