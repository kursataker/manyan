# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | Manyan | speedup | TLC (8) | Manyan (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.22 | 0.47 | 4.7x | 0.97 | 0.11 | 8.6x |
| counters | 823543 | 3.55 | 0.76 | 4.7x | 2.86 | 0.21 | 13.4x |
| liveness | 38880 | 4.31 | 0.18 | 24.2x | 2.98 | 0.17 | 17.1x |
| log | 31501 | 0.66 | 0.05 | 13.1x | 0.58 | 0.03 | 20.5x |
| mutex | 11365 | 0.48 | 0.03 | 17.0x | 0.42 | 0.02 | 17.5x |
| partition | 21147 | 0.92 | 0.17 | 5.4x | 0.69 | 0.06 | 12.2x |
| queue | 183680 | 0.95 | 0.15 | 6.3x | 0.84 | 0.06 | 14.2x |
| recursion | 78125 | 1.22 | 0.24 | 5.1x | 0.84 | 0.07 | 12.1x |
| refinement | 30618 | 0.65 | 0.07 | 8.9x | 0.55 | 0.03 | 16.8x |
| symmetry | 21123 | 1.23 | 0.19 | 6.5x | 0.70 | 0.06 | 12.8x |
