# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | tpp | speedup | TLC (8) | tpp (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.16 | 0.47 | 4.6x | 0.97 | 0.12 | 8.3x |
| counters | 823543 | 3.61 | 0.73 | 4.9x | 2.98 | 0.23 | 13.1x |
| liveness | 38880 | 4.30 | 0.20 | 22.0x | 2.92 | 0.20 | 14.6x |
| log | 31501 | 0.64 | 0.05 | 13.4x | 0.57 | 0.03 | 18.9x |
| mutex | 11365 | 0.47 | 0.03 | 17.5x | 0.43 | 0.02 | 17.4x |
| partition | 21147 | 0.91 | 0.18 | 4.9x | 0.67 | 0.06 | 10.7x |
| queue | 183680 | 0.93 | 0.15 | 6.2x | 0.86 | 0.06 | 13.8x |
| recursion | 78125 | 1.19 | 0.23 | 5.2x | 0.89 | 0.07 | 12.0x |
| refinement | 30618 | 0.64 | 0.11 | 5.9x | 0.56 | 0.05 | 10.8x |
| symmetry | 21123 | 1.22 | 0.18 | 6.9x | 0.68 | 0.07 | 10.1x |
