# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | tpp | speedup | TLC (8) | tpp (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.25 | 0.69 | 3.3x | 1.29 | 0.16 | 7.9x |
| counters | 823543 | 3.79 | 1.05 | 3.6x | 3.16 | 0.32 | 9.8x |
| log | 31501 | 1.02 | 0.06 | 16.5x | 0.99 | 0.04 | 28.2x |
| mutex | 11365 | 0.76 | 0.04 | 19.8x | 0.74 | 0.03 | 25.6x |
| queue | 183680 | 1.29 | 0.20 | 6.4x | 1.23 | 0.08 | 15.0x |
