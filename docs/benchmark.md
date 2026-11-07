# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | tpp | speedup | TLC (8) | tpp (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.20 | 0.47 | 4.6x | 0.93 | 0.12 | 7.7x |
| counters | 823543 | 3.46 | 0.73 | 4.8x | 2.79 | 0.22 | 12.7x |
| liveness | 38880 | 4.23 | 0.21 | 20.1x | 3.04 | 0.21 | 14.3x |
| log | 31501 | 0.66 | 0.05 | 13.7x | 0.57 | 0.03 | 18.7x |
| mutex | 11365 | 0.47 | 0.03 | 17.1x | 0.45 | 0.02 | 18.2x |
| partition | 21147 | 0.91 | 0.18 | 5.0x | 0.70 | 0.06 | 11.7x |
| queue | 183680 | 0.92 | 0.15 | 6.2x | 0.85 | 0.06 | 13.5x |
| recursion | 78125 | 1.20 | 0.23 | 5.3x | 0.90 | 0.08 | 10.8x |
| refinement | 30618 | 0.63 | 0.11 | 5.7x | 0.54 | 0.07 | 8.2x |
| symmetry | 21123 | 1.21 | 0.18 | 6.8x | 0.67 | 0.05 | 12.3x |
