# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | tpp | speedup | TLC (8) | tpp (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.27 | 0.49 | 4.6x | 1.26 | 0.12 | 10.7x |
| counters | 823543 | 3.78 | 0.73 | 5.2x | 3.20 | 0.22 | 14.6x |
| liveness | 38880 | 4.50 | 0.33 | 13.6x | 3.30 | 0.33 | 9.9x |
| log | 31501 | 1.01 | 0.05 | 21.6x | 0.95 | 0.03 | 33.7x |
| mutex | 11365 | 0.77 | 0.03 | 27.2x | 0.76 | 0.02 | 32.1x |
| partition | 21147 | 1.17 | 0.21 | 5.5x | 1.03 | 0.07 | 15.5x |
| queue | 183680 | 1.28 | 0.16 | 8.0x | 1.24 | 0.06 | 19.4x |
| recursion | 78125 | 1.51 | 0.22 | 6.8x | 1.18 | 0.07 | 17.2x |
| refinement | 30618 | 0.98 | 0.14 | 7.0x | 0.97 | 0.05 | 20.9x |
| symmetry | 21123 | 1.45 | 1.32 | 1.1x | 0.96 | 0.28 | 3.5x |
