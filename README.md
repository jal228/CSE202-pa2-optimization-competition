# Optimization Competition

Make `fft_student` in `code.c` as fast as you can while keeping it correct.
You earn up to 20 points based on your speedup, and the single fastest correct
submission in the class earns an extra 1 credit on the final grade.

## Files

| File | Edit? | Purpose |
|---|---|---|
| `code.c` | **yes, submit this** | Your implementation (starts as a copy of the baseline) |
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

- The grader compiles your `code.c` with `gcc -O0`, the same as `make test`,
  together with its own copy of the harness and a secret seed.
- Wrong output at any size, or breaking a rule, earns 0 points.
- Your speedup is the geometric mean of your speedups at N = 64 and N = 100.
- Points out of 20 scale linearly with your speedup, from 0 at 1.0× (no
  improvement) to the full 20 at 4.4× or more, rounded to the nearest whole number:

  ```
  points = round(20 × (speedup − 1) / 3.4),   capped between 0 and 20
  ```

  | Speedup | Points |
  |---|---|
  | 1.0× or less | 0 |
  | 2.0× | 6 |
  | 2.5× | 9 |
  | 3.0× | 12 |
  | 3.5× | 15 |
  | 4.0× | 18 |
  | 4.4× or more | 20 |

- **Competition:** the fastest correct submission in the class gets an extra
  **1 credit on the final grade**, after its code is reviewed by hand. Any
  optimization is allowed: algorithm changes, lookup tables, SIMD intrinsics, and
  inline assembly, as long as you follow the rules below. If the top results are too close to separate reliably, they're re-run
  before a winner is chosen.
- `make sizes` shows roughly what the grader will measure. The grading machine may
  differ a little, so aim a bit above 4.4× if you want the full 20.

## Rules

Breaking any of these rules disqualifies your entry:

- Keep the `fft_student` signature. Declare every helper function and
  file-scope variable `static`.
- Use double precision: your results must match the reference within a relative
  error of 1e-8. `float` will not pass.
- You may only call math functions, `memcpy`/`memset`/`malloc`/`free`, and
  `printf`. No threads, timers, file I/O, or system calls.
- Don't change the compiler's optimization level from inside your code: no
  `#pragma GCC optimize`, `_Pragma("GCC optimize ...")`, or
  `__attribute__((optimize))`. Your code is compiled at `-O0` like everyone else's.
- Don't keep results between calls. Every call gets new input, and every result is checked.
- You may change the algorithm, as long as the output is correct.
- You should be able to explain all your optimizations. 
