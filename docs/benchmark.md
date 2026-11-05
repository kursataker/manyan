# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 3 runs, wall-clock seconds, with 1 worker and with 8.

| spec | distinct states | TLC | tpp | speedup | TLC (8) | tpp (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.35 | 0.50 | 4.7x | 1.28 | 0.14 | 9.2x |
| counters | 823543 | 4.03 | 0.76 | 5.3x | 3.48 | 0.25 | 13.7x |
| log | 31501 | 1.03 | 0.05 | 21.5x | 0.95 | 0.03 | 29.1x |
| mutex | 11365 | 0.76 | 0.03 | 27.2x | 0.73 | 0.03 | 26.9x |
| queue | 183680 | 1.34 | 0.17 | 8.0x | 1.25 | 0.07 | 17.6x |
