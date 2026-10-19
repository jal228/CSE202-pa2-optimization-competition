# Grading uses exactly these flags; changing them here only affects your local runs.
CC     = gcc
CFLAGS = -O0
N     ?= 32
REPS  ?= 5

SRCS = harness.c baseline.c code.c

# Build and run the correctness check and benchmark (rebuilds every time so N changes apply).
test:
	$(CC) $(CFLAGS) -std=gnu11 -Wall -DN=$(N) -DREPS=$(REPS) -o fft_test $(SRCS) -lm
	./fft_test

# Run at the sizes the grader uses.
sizes:
	@for n in 64 100; do $(MAKE) --no-print-directory test N=$$n || exit 1; echo; done

clean:
	rm -f fft_test

.PHONY: test sizes clean
