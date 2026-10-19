// Your competition entry. This is the only file you submit.
//
// Rules (submissions that break them are disqualified):
//   - Keep the fft_student signature from fft.h. Any helper functions or
//     file-scope variables you add must be declared `static`.
//   - Results must match the reference to double precision (relative error <= 1e-8).
//   - Only math, memory (memcpy/memset/malloc/free) and printf functions may be called:
//     no threads, timers, file I/O, or system calls.
//   - No inline assembly, SIMD intrinsics, #pragma, or __attribute__((optimize/constructor)).
//   - Do not keep results between calls; every call gets new input.
//
// Test with:  make test        (or: make sizes)
#include <math.h>
#include "fft.h"

#define PI 3.14159265358979323846

static int get_width(void)  { return N; }
static int get_height(void) { return N; }

// Read a[i][j] into *dest; returns 0 if the index is out of bounds
static int get_element(double a[N][N], int i, int j, double *dest) {
    if (i < 0 || i >= get_height() || j < 0 || j >= get_width())
        return 0;
    *dest = a[i][j];
    return 1;
}

static double normalization(void) {
    return sqrt((double)get_width() * get_height());
}

static double angle(int xWave, int xSpace, int yWave, int ySpace) {
    return 2 * PI * ((1.0 * xWave * xSpace / get_width()) + (1.0 * yWave * ySpace / get_height()));
}

void fft_student(double in[N][N], double re[N][N], double im[N][N], double amp[N][N]) {
    int yWave, xWave, ySpace, xSpace;
    double value;
    // Two outer loops iterate on output data
    for (yWave = 0; yWave < get_height(); yWave++) {
        for (xWave = 0; xWave < get_width(); xWave++) {
            re[yWave][xWave] = 0.0;
            im[yWave][xWave] = 0.0;
            // Two inner loops iterate on input data
            for (xSpace = 0; xSpace < get_width(); xSpace++) {
                for (ySpace = 0; ySpace < get_height(); ySpace++) {
                    // Compute real, imag, and amplitude
                    get_element(in, ySpace, xSpace, &value);
                    re[yWave][xWave] += value * cos(angle(xWave, xSpace, yWave, ySpace)) / normalization();
                    get_element(in, ySpace, xSpace, &value);
                    im[yWave][xWave] -= value * sin(angle(xWave, xSpace, yWave, ySpace)) / normalization();
                    amp[yWave][xWave] = sqrt(re[yWave][xWave] * re[yWave][xWave] +
                                             im[yWave][xWave] * im[yWave][xWave]);
                }
            }
        }
    }
}
