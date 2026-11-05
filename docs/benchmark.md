# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | tpp | speedup | TLC (8) | tpp (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.27 | 0.49 | 4.7x | 1.27 | 0.13 | 9.6x |
| counters | 823543 | 3.89 | 0.75 | 5.2x | 3.43 | 0.24 | 14.3x |
| liveness | 38880 | 4.63 | 0.37 | 12.6x | 3.17 | 0.37 | 8.6x |
| log | 31501 | 1.03 | 0.05 | 22.5x | 0.97 | 0.03 | 31.3x |
| mutex | 11365 | 0.77 | 0.03 | 28.0x | 0.74 | 0.02 | 30.4x |
| partition | 21147 | 1.22 | 0.22 | 5.6x | 1.06 | 0.07 | 14.7x |
| queue | 183680 | 1.29 | 0.16 | 7.8x | 1.28 | 0.07 | 18.6x |
| recursion | 78125 | 1.54 | 0.24 | 6.5x | 1.22 | 0.07 | 17.2x |
| refinement | 30618 | 0.99 | 0.29 | 3.5x | 1.04 | 0.28 | 3.8x |
| symmetry | 21123 | 1.44 | 1.35 | 1.1x | 1.08 | 0.31 | 3.5x |
