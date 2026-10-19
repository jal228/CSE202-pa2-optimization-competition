// Test and benchmark driver for fft_student. Do not edit; the grader uses its own copy.
//
// Usage: ./fft_test [--seed S] [--no-baseline]
//   1. Checks fft_student on fixed and random inputs against an independent
//      high-precision reference.
//   2. Times fft_student on REPS fresh random inputs (each result is checked too)
//      and reports the median, plus the speedup over fft_baseline.
// Grader-only flags: --grade (print a single RESULT line), --baseline (time fft_baseline).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include "fft.h"

#ifndef REPS
#define REPS 5
#endif
#define TOL 1e-8
#define PI_L 3.141592653589793238462643383279502884L

typedef void (*fft_fn)(double[N][N], double[N][N], double[N][N], double[N][N]);

static double cases[6][N][N];
static double inputs[REPS][N][N];
static double inSaved[N][N];
static double outRe[N][N], outIm[N][N], outAmp[N][N];
static long double refRe[N][N], refIm[N][N], tmpRe[N][N], tmpIm[N][N];
static long double twCos[N], twSin[N];
static uint64_t rngState;

static const char *caseNames[6] = {
    "all zeros", "all ones", "impulse at (0,0)", "impulse off-center",
    "random positive", "random signed",
};

// splitmix64: portable, so every machine generates the same inputs for a seed
static double rand_unit(void) {
    uint64_t z = (rngState += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    z ^= z >> 31;
    return (z >> 11) * 0x1.0p-53;
}

static double now(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

// Separable row/column transform in long double with exact twiddle indices.
// Deliberately different from the baseline so a shared mistake can't hide.
static void reference(double in[N][N]) {
    int x, y, u, v, k;
    for (k = 0; k < N; k++) {
        twCos[k] = cosl(2 * PI_L * k / N);
        twSin[k] = sinl(2 * PI_L * k / N);
    }
    for (y = 0; y < N; y++)
        for (u = 0; u < N; u++) {
            long double sr = 0, si = 0;
            for (x = 0; x < N; x++) {
                k = (u * x) % N;
                sr += in[y][x] * twCos[k];
                si -= in[y][x] * twSin[k];
            }
            tmpRe[y][u] = sr;
            tmpIm[y][u] = si;
        }
    for (v = 0; v < N; v++)
        for (u = 0; u < N; u++) {
            long double sr = 0, si = 0;
            for (y = 0; y < N; y++) {
                k = (v * y) % N;
                sr += tmpRe[y][u] * twCos[k] + tmpIm[y][u] * twSin[k];
                si += tmpIm[y][u] * twCos[k] - tmpRe[y][u] * twSin[k];
            }
            refRe[v][u] = sr / N;
            refIm[v][u] = si / N;
        }
}

typedef struct {
    double err;         // max error relative to the largest output magnitude
    int i, j;           // where it occurred
    const char *field;  // which output array
    double got, want;
    int inputModified;
} check_t;

static void compare(double got, long double want, long double scale, int i, int j,
                    const char *field, check_t *c) {
    double d = isnan(got) ? INFINITY : (double)(fabsl(want - got) / scale);
    if (d > c->err) {
        c->err = d;
        c->i = i;
        c->j = j;
        c->field = field;
        c->got = got;
        c->want = (double)want;
    }
}

// Run fn on in[][] with outputs pre-filled with NaN, then check every element.
static check_t run_and_check(fft_fn fn, double in[N][N], double *seconds) {
    check_t c = {0, 0, 0, "", 0, 0, 0};
    long double scale = 0;
    double t0;
    int i, j;

    memcpy(inSaved, in, sizeof inSaved);
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
            outRe[i][j] = outIm[i][j] = outAmp[i][j] = NAN;

    t0 = now();
    fn(in, outRe, outIm, outAmp);
    if (seconds)
        *seconds = now() - t0;

    c.inputModified = memcmp(inSaved, in, sizeof inSaved) != 0;
    reference(inSaved);
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++) {
            if (fabsl(refRe[i][j]) > scale) scale = fabsl(refRe[i][j]);
            if (fabsl(refIm[i][j]) > scale) scale = fabsl(refIm[i][j]);
        }
    if (scale == 0)
        scale = 1;
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++) {
            compare(outRe[i][j], refRe[i][j], scale, i, j, "re", &c);
            compare(outIm[i][j], refIm[i][j], scale, i, j, "im", &c);
            compare(outAmp[i][j], sqrtl(refRe[i][j] * refRe[i][j] + refIm[i][j] * refIm[i][j]),
                    scale, i, j, "amp", &c);
        }
    return c;
}

static int failed(check_t c) {
    return c.inputModified || !(c.err <= TOL);
}

static void explain(check_t c) {
    if (c.inputModified)
        printf("         fft_student modified its input array.\n");
    else
        printf("         first worst mismatch: %s[%d][%d] = %.10g, expected %.10g (relative error %.2e > %.0e)\n",
               c.field, c.i, c.j, c.got, c.want, c.err, TOL);
}

static int cmp_double(const void *a, const void *b) {
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

// Median time of fn over the timing inputs; returns -1 and reports if any result is wrong.
static double time_fn(fft_fn fn, const char *name, int grade) {
    double times[REPS];
    int r;
    for (r = 0; r < REPS; r++) {
        check_t c = run_and_check(fn, inputs[r], &times[r]);
        if (failed(c)) {
            if (grade) {
                printf("RESULT FAIL wrong output on timing run %d\n", r + 1);
            } else {
                printf("  [FAIL] %s gave a wrong result on timing run %d\n", name, r + 1);
                explain(c);
                printf("         (Results must not depend on earlier calls.)\n");
            }
            return -1;
        }
    }
    qsort(times, REPS, sizeof times[0], cmp_double);
    return times[REPS / 2];
}

int main(int argc, char **argv) {
    int grade = 0, baselineOnly = 0, skipBaseline = 0, i, j, r, k;
    unsigned long long seed = (unsigned long long)time(NULL);
    fft_fn fn;
    const char *name;
    double tStudent, tBase;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--grade")) grade = 1;
        else if (!strcmp(argv[i], "--baseline")) baselineOnly = 1;
        else if (!strcmp(argv[i], "--no-baseline")) skipBaseline = 1;
        else if (!strcmp(argv[i], "--seed") && i + 1 < argc) seed = strtoull(argv[++i], NULL, 10);
        else {
            fprintf(stderr, "usage: %s [--seed S] [--no-baseline]\n", argv[0]);
            return 2;
        }
    }
    fn = baselineOnly ? fft_baseline : fft_student;
    name = baselineOnly ? "fft_baseline" : "fft_student";
    rngState = seed;

    // Correctness inputs
    for (k = 0; k < 6; k++)
        for (i = 0; i < N; i++)
            for (j = 0; j < N; j++)
                cases[k][i][j] = k == 1 ? 1.0
                               : k == 4 ? 1000.0 * rand_unit()
                               : k == 5 ? 2000.0 * rand_unit() - 1000.0
                               : 0.0;
    cases[2][0][0] = 1.0;
    cases[3][N / 3][(N / 2 + 1) % N] = 1.0;
    // Timing inputs: fresh data for every run
    for (r = 0; r < REPS; r++)
        for (i = 0; i < N; i++)
            for (j = 0; j < N; j++)
                inputs[r][i][j] = 2000.0 * rand_unit() - 1000.0;

    if (!grade)
        printf("N = %d, timing runs = %d, seed = %llu\n\nCorrectness:\n", N, REPS, seed);
    for (k = 0; k < 6; k++) {
        check_t c = run_and_check(fn, cases[k], NULL);
        if (failed(c)) {
            if (grade) {
                printf("RESULT FAIL wrong output for input '%s'\n", caseNames[k]);
            } else {
                printf("  [FAIL] %-20s\n", caseNames[k]);
                explain(c);
                printf("\nFix correctness first; no timing is done until all tests pass.\n");
            }
            return 1;
        }
        if (!grade)
            printf("  [PASS] %-20s (max relative error %.1e)\n", caseNames[k], c.err);
    }

    if (!grade)
        printf("\nTiming:\n");
    tStudent = time_fn(fn, name, grade);
    if (tStudent < 0)
        return 1;
    if (grade) {
        printf("RESULT OK %.9f\n", tStudent);
        return 0;
    }
    printf("  %-13s median %10.6f s\n", name, tStudent);
    if (!skipBaseline && !baselineOnly) {
        tBase = time_fn(fft_baseline, "fft_baseline", 0);
        if (tBase < 0)
            return 1;
        printf("  %-13s median %10.6f s\n", "fft_baseline", tBase);
        printf("\n  Speedup: %.2fx\n", tBase / tStudent);
    }
    return 0;
}
