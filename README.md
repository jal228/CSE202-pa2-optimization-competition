# Optimization Competition

Make `fft_student` in `student.c` as fast as you can while keeping it correct.
The fastest correct submission earns extra credit.

## Files

| File | Edit? | Purpose |
|---|---|---|
| `student.c` | **yes, submit this** | Your implementation (starts as a copy of the baseline) |
| `fft.h` | no | Function signatures and what the function must compute |
| `baseline.c` | no | Unoptimized version your speedup is measured against |
| `harness.c` | no | Correctness test and timer |
| `Makefile` | no | `make test`, `make sizes` |

## Testing your code

```
make test            # check correctness and time at N=32
make test N=64       # any size
make sizes           # the sizes used for grading (64 and 100)
```

The test first checks your output against an independent high-precision
reference on six inputs (zeros, ones, impulses, random). If they all pass, it
times your function on 5 new random inputs (checking each result too) and
reports the median time and the speedup over the baseline.

Each run uses a new random seed, which it prints. To repeat a run exactly,
use `./fft_test --seed <number>`.

## How you are graded

- The grader compiles your `student.c` with `gcc -O0`, the same as `make test`,
  together with its own copy of the harness and a secret seed.
- Wrong output at any size means no speed score.
- Your score is the geometric mean of your speedups at N = 64 and N = 100.
- The single fastest correct entry gets extra credit after its code is reviewed by hand.

## Rules

Breaking any of these rules disqualifies your entry:

- Keep the `fft_student` signature. Declare every helper function and
  file-scope variable `static`.
- Use double precision: your results must match the reference within a relative
  error of 1e-8. `float` will not pass.
- You may only call math functions, `memcpy`/`memset`/`malloc`/`free`, and
  `printf`. No threads, timers, file I/O, or system calls.
- No inline assembly, SIMD intrinsics, `#pragma`, or
  `__attribute__((optimize))` / `__attribute__((constructor))`.
- Don't keep results between calls. Every call gets new input, and every result is checked.
- You may change the algorithm, as long as the output is correct.
