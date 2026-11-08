# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | tpp | speedup | TLC (8) | tpp (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.20 | 0.44 | 5.0x | 0.99 | 0.12 | 8.5x |
| counters | 823543 | 3.43 | 0.74 | 4.6x | 3.09 | 0.22 | 14.2x |
| liveness | 38880 | 4.26 | 0.18 | 23.5x | 2.92 | 0.18 | 15.8x |
| log | 31501 | 0.64 | 0.05 | 13.0x | 0.58 | 0.03 | 20.1x |
| mutex | 11365 | 0.46 | 0.03 | 17.0x | 0.43 | 0.02 | 17.8x |
| partition | 21147 | 0.90 | 0.17 | 5.4x | 0.68 | 0.06 | 12.0x |
| queue | 183680 | 0.92 | 0.15 | 6.3x | 0.82 | 0.06 | 13.9x |
| recursion | 78125 | 1.21 | 0.23 | 5.2x | 0.81 | 0.07 | 11.6x |
| refinement | 30618 | 0.63 | 0.07 | 9.1x | 0.56 | 0.03 | 17.4x |
| symmetry | 21123 | 1.20 | 0.18 | 6.7x | 0.69 | 0.06 | 12.4x |
