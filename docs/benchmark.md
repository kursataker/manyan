# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | Manyan | speedup | TLC (8) | Manyan (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.30 | 0.48 | 4.8x | 1.06 | 0.13 | 8.0x |
| counters | 823543 | 3.76 | 0.79 | 4.7x | 3.20 | 0.27 | 12.0x |
| liveness | 38880 | 4.55 | 0.22 | 20.7x | 3.08 | 0.23 | 13.2x |
| log | 31501 | 0.73 | 0.06 | 13.3x | 0.69 | 0.04 | 17.8x |
| mutex | 11365 | 0.55 | 0.03 | 17.0x | 0.48 | 0.03 | 16.3x |
| partition | 21147 | 1.08 | 0.20 | 5.5x | 0.81 | 0.07 | 11.2x |
| queue | 183680 | 1.07 | 0.16 | 6.5x | 1.00 | 0.08 | 13.2x |
| recursion | 78125 | 1.32 | 0.26 | 5.0x | 0.98 | 0.08 | 11.6x |
| refinement | 30618 | 0.72 | 0.08 | 8.5x | 0.63 | 0.04 | 14.8x |
| symmetry | 21123 | 1.39 | 0.20 | 6.8x | 0.83 | 0.07 | 11.9x |
