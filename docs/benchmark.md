# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | tpp | speedup | TLC (8) | tpp (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.28 | 0.57 | 4.0x | 1.24 | 0.14 | 8.9x |
| counters | 823543 | 3.83 | 0.90 | 4.3x | 3.18 | 0.30 | 10.7x |
| log | 31501 | 1.01 | 0.06 | 18.2x | 0.93 | 0.03 | 28.3x |
| mutex | 11365 | 0.76 | 0.03 | 24.5x | 0.74 | 0.03 | 28.2x |
| queue | 183680 | 1.30 | 0.18 | 7.1x | 1.17 | 0.08 | 14.7x |
