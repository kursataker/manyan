# Benchmark

<!-- results -->

Measured on arm64 Darwin 25.5.0, best of 1 runs, wall-clock seconds; TLC with `-workers 1`.

| spec | distinct states | TLC | tpp | speedup |
|---|---:|---:|---:|---:|
| counters | 823543 | 3.94 | 2.34 | 1.7x |
| mutex | 11365 | 0.77 | 0.07 | 11.0x |
| queue | 183680 | 1.29 | 0.37 | 3.5x |
