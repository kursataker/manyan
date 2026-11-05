# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | tpp | speedup | TLC (8) | tpp (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.26 | 1.16 | 1.9x | 1.29 | 0.26 | 5.0x |
| counters | 823543 | 3.83 | 1.66 | 2.3x | 3.16 | 0.51 | 6.2x |
| log | 31501 | 1.03 | 0.09 | 11.1x | 0.94 | 0.04 | 21.6x |
| mutex | 11365 | 0.76 | 0.05 | 13.9x | 0.75 | 0.03 | 22.3x |
| queue | 183680 | 1.29 | 0.26 | 5.0x | 1.20 | 0.10 | 12.2x |
