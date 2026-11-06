# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | tpp | speedup | TLC (8) | tpp (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.36 | 0.50 | 4.7x | 1.36 | 0.14 | 9.7x |
| counters | 823543 | 3.90 | 0.77 | 5.0x | 3.33 | 0.26 | 12.8x |
| liveness | 38880 | 4.84 | 0.25 | 19.1x | 3.40 | 0.25 | 13.4x |
| log | 31501 | 1.01 | 0.05 | 21.0x | 0.94 | 0.03 | 28.9x |
| mutex | 11365 | 0.77 | 0.03 | 27.5x | 0.75 | 0.02 | 30.1x |
| partition | 21147 | 1.19 | 0.22 | 5.4x | 1.06 | 0.07 | 14.9x |
| queue | 183680 | 1.28 | 0.16 | 8.0x | 1.25 | 0.06 | 19.6x |
| recursion | 78125 | 1.51 | 0.23 | 6.7x | 1.20 | 0.07 | 16.0x |
| refinement | 30618 | 0.99 | 0.10 | 9.5x | 0.95 | 0.04 | 23.5x |
| symmetry | 21123 | 1.42 | 0.19 | 7.4x | 1.01 | 0.06 | 17.7x |
