# tpp

TLA⁺⁺ (TLA Plus Plus) is a model checker for TLA⁺ written in C11. It reads
the same specifications and model configurations as TLC and produces same
outcome, 4.7 to 28.7 times faster with one worker (`make release`).

```
$ tpp -workers auto MySpec
Finished computing initial states: 1 distinct state generated.
Error: Invariant Safe is violated.
Error: The behavior up to this point is:
State 1: <Initial predicate>
/\ x = 0
/\ y = 0

State 2: <Next line 7, col 9 to line 7, col 30 of module MySpec>
/\ x = 1
/\ y = 0
...
```

## Why

TLC is the reference model checker for TLA⁺, and tpp is tested
against. tpp keeps TLC's semantics and output, so it can stand in for TLC
in scripts and CI, and it starts quickly and spends less time on each
state:

- one C file and one header, with no runtime or dependencies;
- values interned in arenas and states stored as compact encodings;
- fingerprints hashed straight from values;
- with `-workers N`, a parallel breadth-first search whose output is
  identical to a one-worker run, including which violation it reports and
  its trace (TLC's traces vary between multi-worker runs).

## Performance

Wall-clock seconds, best of 3 runs, on an Apple M2 (4 performance and 4
efficiency cores). TLC is tla2tools 1.7.4. Every run prints the same
report as TLC with one worker.

| spec | distinct states | TLC | tpp | speedup | TLC (8) | tpp (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.29 | 0.49 | 4.7x | 1.28 | 0.12 | 10.4x |
| counters | 823543 | 3.86 | 0.73 | 5.3x | 3.30 | 0.23 | 14.3x |
| log | 31501 | 1.01 | 0.05 | 22.0x | 0.96 | 0.03 | 34.1x |
| mutex | 11365 | 0.77 | 0.03 | 28.7x | 0.76 | 0.02 | 30.7x |
| queue | 183680 | 1.28 | 0.16 | 8.0x | 1.21 | 0.07 | 18.3x |

`make bench` reproduces these numbers, with the `make release` build, in [docs/benchmark.md](docs/benchmark.md);
[docs/optimizations.md](docs/optimizations.md) records how they were reached.

## Getting started

Each [release](https://github.com/soasme/tlaplusplus/releases) has `tpp`
built for macOS, Linux and Windows. To build it instead:

```sh
make                          # builds ./tpp; needs a C11 compiler
make release                  # faster ./tpp: profile-guided, trained on tests/spec
./tpp -config MySpec.cfg MySpec
./tpp -workers auto MySpec    # one worker per core
make install                  # PREFIX=/usr/local by default
```

See [the manual](docs/manual.md) for the options, the configuration file,
exit statuses and the C API.

## What it supports

- The TLA⁺ language as TLC evaluates it: operators, functions, records,
  sequences, sets (including lazily evaluated infinite ones), `CHOOSE`,
  `LET`, recursion, `EXTENDS`, parameterized `INSTANCE`, nested modules,
  labels and subexpression names (`Op(a)!lab!:!2`), SANY's level checks.
- Standard modules: Naturals, Integers, Sequences, FiniteSets, TLC, Bags,
  Randomization.
- Model configuration: `CONSTANT` values, model values and `<-`
  substitutions, `INIT`/`NEXT` or `SPECIFICATION`, `INVARIANT`,
  `PROPERTY`, `CONSTRAINT`, `ACTION_CONSTRAINT`, `SYMMETRY`, `VIEW`,
  `CHECK_DEADLOCK`.
- Checking: assumptions, invariants, action properties, deadlock, and
  temporal properties under weak and strong fairness, with TLC's
  counterexamples; random simulation (`-simulate`), drawing the same
  behaviors as TLC for a seed.
- TLC's error messages and exit statuses for evaluation errors, parse
  errors and configuration errors.

Not yet supported: generation mode, writing simulated behaviors to
files, checkpoints, distributed checking. Real numbers and `\AA`/`\EE`
give TLC's errors, as TLC cannot check them either. The proof language is
parsed and skipped.

## How it is tested

`tpp` follows [the tpp TLA⁺ Standard](docs/standards/tla2.md),
with TLC 2.19 as the oracle for every choice the language leaves open.
The tests follow it:

- `tests/spec`: over 200 models, each with TLC's output and exit status.
  `make test-spec-oracle` runs TLC on every one of them as well, so a test
  can't drift from what TLC does. `tests/spec/tlc` holds models imported
  from TLC's own test suite.
- `make test` runs the suite with one worker and with four;
  `make sanitize` runs it under UndefinedBehaviorSanitizer (with Guard
  Malloc on macOS, AddressSanitizer elsewhere).
- `make fuzz` runs mutated models of the suite under the sanitizers, to
  find crashes; `make fuzz-oracle` compares what tpp and TLC print for
  random expressions, and `make fuzz-sim` for random simulations of the
  suite's models, to find differences. What they find becomes a test.
- `tests/tla` models parts of tpp's algorithms in TLA⁺, for TLC and tpp to
  check.

## Layout

```
tpp.c, tpp.h           the checker and its C API
docs/standards/        the language standard tpp conforms to
docs/manual.md         how to use tpp
docs/benchmark.md      tpp against TLC
docs/optimizations.md  what made tpp fast, and what did not
tests/spec/            conformance tests, by chapter of the standard
tests/tla/             TLA⁺ models of tpp's algorithms
bench/                 benchmark specs, compare.py (against TLC), quick.sh (two builds)
CHANGELOG.md           what each release changed; releasing is tagging vX.Y.Z
```
