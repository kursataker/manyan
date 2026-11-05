# tpp

tpp (TLA Plus Plus) is a model checker for TLA+ written in C11. It reads
the same specifications and model configurations as TLC and prints the
same report, the same traces and counterexamples, and the same state
counts, often several times faster.

```
$ tpp -workers auto MySpec
tpp version 0.1.0
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

TLC is the reference model checker for TLA+, and the oracle tpp is tested
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
| closure | 65536 | 2.26 | 1.16 | 1.9x | 1.29 | 0.26 | 5.0x |
| counters | 823543 | 3.83 | 1.66 | 2.3x | 3.16 | 0.51 | 6.2x |
| log | 31501 | 1.03 | 0.09 | 11.1x | 0.94 | 0.04 | 21.6x |
| mutex | 11365 | 0.76 | 0.05 | 13.9x | 0.75 | 0.03 | 22.3x |
| queue | 183680 | 1.29 | 0.26 | 5.0x | 1.20 | 0.10 | 12.2x |

`make bench` reproduces these numbers in [docs/benchmark.md](docs/benchmark.md).

## Getting started

```sh
make                          # builds ./tpp; needs a C11 compiler
./tpp -config MySpec.cfg MySpec
./tpp -workers auto MySpec    # one worker per core
make install                  # PREFIX=/usr/local by default
```

See [the manual](docs/manual.md) for the options, the configuration file,
exit statuses and the C API.

## What it supports

- The TLA+ language as TLC evaluates it: operators, functions, records,
  sequences, sets (including lazily evaluated infinite ones), `CHOOSE`,
  `LET`, recursion, `EXTENDS`, parameterized `INSTANCE`, nested modules.
- Standard modules: Naturals, Integers, Sequences, FiniteSets, TLC, Bags,
  Randomization.
- Model configuration: `CONSTANT` values, model values and `<-`
  substitutions, `INIT`/`NEXT` or `SPECIFICATION`, `INVARIANT`,
  `PROPERTY`, `CONSTRAINT`, `ACTION_CONSTRAINT`, `SYMMETRY`, `VIEW`,
  `CHECK_DEADLOCK`.
- Checking: assumptions, invariants, action properties, deadlock, and
  temporal properties under weak and strong fairness, with TLC's
  counterexamples.
- TLC's error messages and exit statuses for evaluation errors, parse
  errors and configuration errors.

Not yet supported: real numbers, `\AA`/`\EE`, simulation and generation
modes, checkpoints, distributed checking. The proof language is parsed and
skipped.

## How it is tested

What tpp must do is written down first, in
[the tpp TLA+ Standard](docs/standards/tla2.md), with TLC 2.19 as the
oracle for every choice the language leaves open. The tests follow it:

- `tests/spec`: over 200 models, each with TLC's output and exit status.
  `make test-spec-oracle` runs TLC on every one of them as well, so a test
  can't drift from what TLC does. `tests/spec/tlc` holds models imported
  from TLC's own test suite.
- `make test` runs the suite with one worker and with four;
  `make sanitize` runs it under UndefinedBehaviorSanitizer (with Guard
  Malloc on macOS, AddressSanitizer elsewhere).
- `tests/tla` models parts of tpp's algorithms in TLA+, for TLC and tpp to
  check.

## Layout

```
tpp.c, tpp.h           the checker and its C API
docs/standards/        the language standard tpp conforms to
docs/manual.md         how to use tpp
docs/benchmark.md      tpp against TLC
tests/spec/            conformance tests, by chapter of the standard
tests/tla/             TLA+ models of tpp's algorithms
bench/                 benchmark specs and bench/compare.py
```
