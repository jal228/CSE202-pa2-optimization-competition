#ifndef FFT_H
#define FFT_H

// Problem size. Set at compile time with -DN=<size>; the grader uses several sizes.
#ifndef N
#define N 32
#endif

// Compute the 2D transform of in[][] into re[][], im[][] and amp[][].
//   re[v][u]  =  (1/N) * sum over y,x of in[y][x] * cos(2*PI*(u*x + v*y) / N)
//   im[v][u]  = -(1/N) * sum over y,x of in[y][x] * sin(2*PI*(u*x + v*y) / N)
//   amp[v][u] =  sqrt(re[v][u]^2 + im[v][u]^2)
// Every element of re, im and amp must be written. in[][] must not be modified.
void fft_student(double in[N][N], double re[N][N], double im[N][N], double amp[N][N]);

// The unoptimized version your speedup is measured against (baseline.c).
void fft_baseline(double in[N][N], double re[N][N], double im[N][N], double amp[N][N]);

#endif
