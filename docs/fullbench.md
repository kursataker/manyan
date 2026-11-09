# Full benchmark

`make full-bench` checks the models of
[tlaplus/Examples](https://github.com/tlaplus/Examples) with tpp and with
TLC, and writes this report. `make bench` ([benchmark](benchmark.md)) is the
quick one for day-to-day work; this one takes hours and asks a stronger
question: does tpp print what TLC prints on specs it was not written for?

Examples, CommunityModules and TLAPS are pinned in the Makefile. Each model
runs once with one worker, killed past 15 minutes or 4 GB. TLC's results are
cached under `.obj` and the diffs of the models that differ are written to
`.obj/fullbench.diff`. `ONLY=DieHard` runs the models whose path contains
DieHard; `MAX_RUNTIME=00:00:30` keeps those recorded to run in 30 seconds.

SpanningTree/SpanTreeRandom draws its graph with `RandomElement` in a
constant, which TLC draws anew each run, so its output is TLC's of one run.

<!-- results -->

tlaplus/Examples at ca168055339d, the exhaustive-search models recorded to run in at most 00:10:00. Measured on arm64 Darwin 25.5.0, one run, wall-clock seconds, one worker.

3 models: 3 same. Where tpp prints what TLC prints, it is 43.6x as fast (geometric mean).

| model | distinct states | TLC | tpp | speedup | result |
|---|---:|---:|---:|---:|---|
| DieHard/DieHard | 14 | 0.35 | 0.01 | 68.6x | same |
| DieHard/MCDieHarder | 14 | 0.35 | 0.01 | 66.8x | same |
| DieHard/MCDieHardest | 11566 | 0.67 | 0.04 | 18.1x | same |
