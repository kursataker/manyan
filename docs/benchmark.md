# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | tpp | speedup | TLC (8) | tpp (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.30 | 0.48 | 4.8x | 1.05 | 0.13 | 8.1x |
| counters | 823543 | 3.74 | 0.78 | 4.8x | 3.14 | 0.26 | 12.2x |
| liveness | 38880 | 4.50 | 0.20 | 22.4x | 3.06 | 0.20 | 15.3x |
| log | 31501 | 0.70 | 0.05 | 12.9x | 0.62 | 0.03 | 17.8x |
| mutex | 11365 | 0.52 | 0.04 | 14.5x | 0.50 | 0.03 | 17.4x |
| partition | 21147 | 0.98 | 0.20 | 4.9x | 0.70 | 0.07 | 9.9x |
| queue | 183680 | 1.01 | 0.16 | 6.3x | 0.91 | 0.07 | 13.0x |
| recursion | 78125 | 1.28 | 0.24 | 5.3x | 0.84 | 0.07 | 11.6x |
| refinement | 30618 | 0.68 | 0.08 | 8.6x | 0.60 | 0.04 | 14.8x |
| symmetry | 21123 | 1.29 | 0.21 | 6.3x | 0.77 | 0.07 | 11.4x |
