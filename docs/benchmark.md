# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | tpp | speedup | TLC (8) | tpp (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.29 | 0.49 | 4.7x | 1.28 | 0.12 | 10.4x |
| counters | 823543 | 3.86 | 0.73 | 5.3x | 3.30 | 0.23 | 14.3x |
| log | 31501 | 1.01 | 0.05 | 22.0x | 0.96 | 0.03 | 34.1x |
| mutex | 11365 | 0.77 | 0.03 | 28.7x | 0.76 | 0.02 | 30.7x |
| queue | 183680 | 1.28 | 0.16 | 8.0x | 1.21 | 0.07 | 18.3x |
