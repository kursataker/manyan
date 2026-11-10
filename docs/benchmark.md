# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | Manyan | speedup | TLC (8) | Manyan (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.24 | 0.47 | 4.8x | 0.98 | 0.12 | 8.3x |
| counters | 823543 | 3.60 | 0.77 | 4.7x | 3.15 | 0.23 | 13.7x |
| liveness | 38880 | 4.58 | 0.19 | 24.0x | 3.02 | 0.19 | 16.0x |
| log | 31501 | 0.67 | 0.05 | 13.3x | 0.60 | 0.03 | 18.2x |
| mutex | 11365 | 0.48 | 0.03 | 16.7x | 0.50 | 0.03 | 18.9x |
| partition | 21147 | 0.96 | 0.18 | 5.2x | 0.76 | 0.07 | 11.3x |
| queue | 183680 | 0.96 | 0.16 | 6.2x | 0.86 | 0.09 | 10.1x |
| recursion | 78125 | 1.25 | 0.24 | 5.2x | 0.90 | 0.08 | 10.8x |
| refinement | 30618 | 0.66 | 0.08 | 8.6x | 0.60 | 0.04 | 16.3x |
| symmetry | 21123 | 1.25 | 0.19 | 6.6x | 0.73 | 0.06 | 12.0x |
