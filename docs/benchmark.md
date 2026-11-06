# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | tpp | speedup | TLC (8) | tpp (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.25 | 0.46 | 4.9x | 1.28 | 0.12 | 10.7x |
| counters | 823543 | 3.79 | 0.73 | 5.2x | 3.56 | 0.23 | 15.5x |
| liveness | 38880 | 4.59 | 0.20 | 22.8x | 3.19 | 0.20 | 15.7x |
| log | 31501 | 1.01 | 0.05 | 21.3x | 0.96 | 0.03 | 32.2x |
| mutex | 11365 | 0.77 | 0.03 | 28.9x | 0.74 | 0.02 | 30.4x |
| partition | 21147 | 1.19 | 0.20 | 6.1x | 1.02 | 0.06 | 16.4x |
| queue | 183680 | 1.30 | 0.15 | 8.9x | 1.17 | 0.06 | 19.3x |
| recursion | 78125 | 1.52 | 0.23 | 6.8x | 1.26 | 0.07 | 16.9x |
| refinement | 30618 | 0.99 | 0.11 | 9.2x | 0.97 | 0.04 | 22.2x |
| symmetry | 21123 | 1.43 | 0.18 | 7.8x | 1.05 | 0.07 | 14.6x |
