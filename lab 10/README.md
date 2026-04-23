# CENG356 Lab 10 - 60-Minute Microarchitectural Side-Channel Audit

This folder contains the following lab programs:

- `branch_predictor_audit.c`: pure C version
- `cache_timing_sensor.c`: pure C cache hit/miss timing sensor

## Objective

This lab now contains two pure C microarchitectural timing experiments:

1. Part 1: a branch predictor training and mismatch benchmark.
2. Part 2: a cache hit versus cache miss timing sensor.

## Program Summary

### Part 1: branch_predictor_audit.c

1. Runs a branch with the `if` path taken 99 times.
2. On the 100th branch, either:
   - keeps taking the `if` path for the trained case, or
   - forces the `else` path for the mismatched case.
3. Repeats that 100-branch pattern many times in one timed window.
4. Reports:
   - average cycles for the trained window,
   - average cycles for the mismatched window,
   - the observed mismatch penalty in cycles,
   - median versions of the same measurements.

### Part 2: cache_timing_sensor.c

1. Allocates a 64 MiB array, which is large enough to exceed typical CPU cache capacity.
2. Measures a cache hit by re-accessing a warmed memory location.
3. Measures a cache miss by flushing and then accessing a distant memory location.
4. Reports:
   - average cache hit time,
   - average cache miss time,
   - hit/miss ratio,
   - miss/hit slowdown,
   - median hit and miss times.

## Build on Windows with MinGW

### Part 1

```powershell
gcc -O2 -std=c11 -Wall -Wextra -o branch_predictor_audit_c.exe branch_predictor_audit.c
```

### Part 2

```powershell
gcc -O2 -std=c11 -Wall -Wextra -o cache_timing_sensor.exe cache_timing_sensor.c
```

## Run

### Part 1

```powershell
.\branch_predictor_audit_c.exe
```

### Part 2

```powershell
.\cache_timing_sensor.exe
```

## Deliverables

### Part 1

The main value to highlight in your deliverable is:

`Observed mismatch penalty: X.XX cycles`

That number estimates the extra cost of forcing the wrong branch direction after predictor training. The exact value depends on your CPU, current system load, thermal state, and other microarchitectural noise sources.

### Part 2

The cache timing program prints both cycles and approximate nanoseconds. The main values to report are:

- cache hit time,
- cache miss time,
- hit/miss ratio.

An example interpretation is `20 ns` for a hit versus `123 ns` for a miss, with a hit/miss ratio of about `0.16`.

## Notes

- Both programs target x86/x86-64 systems with `RDTSC` support.
- Results vary from run to run.
- Small penalties are still valid if they are repeatable on your machine.
- Background activity, CPU frequency changes, and thermal throttling can affect timing.
