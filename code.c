// Your optimized implementation. This is the only file you submit.
//
// Rules (submissions that break them are disqualified):
//   - Keep the fft_student signature from fft.h. Any helper functions or
//     file-scope variables you add must be declared `static`.
//   - Results must match the reference to double precision (relative error <= 1e-8).
//   - Only math, memory (memcpy/memset/malloc/free) and printf functions may be called:
//     no threads, timers, file I/O, or system calls.
//   - Do not keep results between calls; every call gets new input.
//   - No #pragma GCC optimize or __attribute__((optimize)): your code is compiled at -O0.
//   - Individual work only: do not discuss your solution, work in pairs, or share code.
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

static double cosT[N], sinT[N];       //cos/sin(2*pi*k/N) for k = 0..N-1
static double tRe[N][N], tIm[N][N];   //the rows after  first pass

void fft_student(double in[N][N], double re[N][N], double im[N][N], double amp[N][N]) {
    int x, y, u, v, k;
    double sr, si, c, s, a, b;
    double *row, *rowRe, *rowIm, *outRe, *outIm;

    //Lookup table, angles only take N distinct values, compute
    //cos/sin once per call instead of inside the loops
    for (k = 0; k < N; k++) {
        cosT[k] = cos(2 * PI * k / N);
        sinT[k] = sin(2 * PI * k / N);
    }

    //Pass 1, transform each row along x
    for (y = 0; y < N; y++) {
        row = in[y];
        for (u = 0; u < N; u++) {
            sr = si = 0;
            k = 0;
            for (x = 0; x < N; x++) {
                sr += row[x] * cosT[k];
                si -= row[x] * sinT[k];
                k += u;
                if (k >= N) k -= N;
            }
            tRe[y][u] = sr;
            tIm[y][u] = si;
        }
    }

    //Pass 2, transform each column along y, whole row at a time
    for (v = 0; v < N; v++) {
        outRe = re[v];
        outIm = im[v];
        for (u = 0; u < N; u++)
            outRe[u] = outIm[u] = 0;
        k = 0;
        for (y = 0; y < N; y++) {
            c = cosT[k];
            s = sinT[k];
            rowRe = tRe[y];
            rowIm = tIm[y];
            for (u = 0; u < N; u++) {
                outRe[u] += rowRe[u] * c + rowIm[u] * s;
                outIm[u] += rowIm[u] * c - rowRe[u] * s;
            }
            k += v;
            if (k >= N) k -= N;
        }
        //Normalize when at the end and compute amplitude
        for (u = 0; u < N; u++) {
            a = outRe[u] / N;
            b = outIm[u] / N;
            outRe[u] = a;
            outIm[u] = b;
            amp[v][u] = sqrt(a * a + b * b);
        }
    }
}
