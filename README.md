# TLA⁺⁺

TLA⁺⁺ (TLAPlusPlus) is a model checker for TLA⁺ written in C11. It reads
the same specifications and model configurations as TLC and produces same
outcome, 1.1 to 27.2 times faster with one worker.

![logo](docs/logo.png)

\* Write it as `TLA++` when superscript is unavailable.

## Getting started

Each [release](https://github.com/soasme/tlaplusplus/releases) has `tpp`
built for macOS, Linux and Windows. To build it instead:

```sh
make                          # builds ./tpp; needs a C11 compiler
./tpp examples/DieHard        # checks examples/DieHard.tla with examples/DieHard.cfg
./tpp -workers auto examples/DieHard   # one worker per core
make install PREFIX=~/.local  # installs ~/.local/bin/tpp
```

With python3, `make` builds `tpp` profile-guided, trained on the
conformance suite, which makes it 10 to 25% faster.

See [the manual](docs/manual.md) for the options, the configuration file,
exit statuses and the C API.

## Example Usage

```
$ ./tpp examples/DieHard
tpp version 0.6.0
Finished computing initial states: 1 distinct state generated.
Error: Invariant NotSolved is violated.
Error: The behavior up to this point is:
State 1: <Initial predicate>
/\ big = 0
/\ small = 0

State 2: <Next line 9, col 12 to line 9, col 37 of module DieHard>
/\ big = 5
/\ small = 0
...
```

## Why

TLC is the reference model checker for TLA⁺, and `tpp` is tested
against. `tpp` keeps TLC's semantics and output, so it can stand in for TLC
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
efficiency cores). TLC is tla2tools 1.8.0 (d8fb231). Every run prints the same
report as TLC with one worker.

| spec | distinct states | TLC | tpp | speedup | TLC (8) | tpp (8) | speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| closure | 65536 | 2.16 | 0.47 | 4.6x | 0.97 | 0.12 | 8.3x |
| counters | 823543 | 3.61 | 0.73 | 4.9x | 2.98 | 0.23 | 13.1x |
| liveness | 38880 | 4.30 | 0.20 | 22.0x | 2.92 | 0.20 | 14.6x |
| log | 31501 | 0.64 | 0.05 | 13.4x | 0.57 | 0.03 | 18.9x |
| mutex | 11365 | 0.47 | 0.03 | 17.5x | 0.43 | 0.02 | 17.4x |
| partition | 21147 | 0.91 | 0.18 | 4.9x | 0.67 | 0.06 | 10.7x |
| queue | 183680 | 0.93 | 0.15 | 6.2x | 0.86 | 0.06 | 13.8x |
| recursion | 78125 | 1.19 | 0.23 | 5.2x | 0.89 | 0.07 | 12.0x |
| refinement | 30618 | 0.64 | 0.11 | 5.9x | 0.56 | 0.05 | 10.8x |
| symmetry | 21123 | 1.22 | 0.18 | 6.9x | 0.68 | 0.07 | 10.1x |

`make bench` reproduces these numbers in [docs/benchmark.md](docs/benchmark.md);
[docs/optimizations.md](docs/optimizations.md) records how they were reached.

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
  `ALIAS`, `POSTCONDITION`, `CHECK_DEADLOCK`.
- Checking: assumptions, invariants, action properties, deadlock, and
  temporal properties under weak and strong fairness, with TLC's
  counterexamples; random simulation (`-simulate`), drawing the same
  behaviors as TLC for a seed, and writing them to files (`file=`);
  TLC's `-continue`, `-difftrace`, `-view`, `-userFile` and `-dump dot`.
- TLC's error messages and exit statuses for evaluation errors, parse
  errors and configuration errors.

Not yet supported: TLC's `-terse`, `-maxSetSize`, `-dfid`, `-coverage`,
`-tool` and `-generateSpecTE`, checkpoints, distributed checking (see
the [manual](docs/manual.md#limitations)). Real numbers and `\AA`/`\EE`
give TLC's errors, as TLC cannot check them either. The proof language is
parsed and skipped.

## How it is tested

`tpp` follows [the tpp TLA⁺ Standard](docs/standards/tla2.md),
with TLC 1.8.0 as the oracle for every choice the language leaves open.
The tests follow it; they need python3, and those that run TLC also need
Java (they download tla2tools 1.8.0 themselves):

- `tests/spec`: over 500 models, each with TLC's output and exit status.
  `make test-spec-oracle` runs TLC on every one of them as well, so a test
  can't drift from what TLC does. `tests/spec/tlc` holds models imported
  from TLC's own test suite.
- `make test` runs the suite with one worker and with four;
  `make sanitize` runs it under UndefinedBehaviorSanitizer (with Guard
  Malloc on macOS, AddressSanitizer elsewhere).
- `make fuzz` runs mutated models of the suite under the sanitizers, to
  find crashes; `make fuzz-oracle` compares what `tpp` and TLC print for
  random expressions, and `make fuzz-sim` for random simulations of the
  suite's models, to find differences. What they find becomes a test.
- `tests/tla` models parts of `tpp`'s algorithms in TLA⁺, for TLC and `tpp` to
  check.

## Layout

```
tpp.c, tpp.h           the checker and its C API
docs/standards/        the language standard tpp conforms to
docs/manual.md         how to use tpp
docs/benchmark.md      tpp against TLC
docs/optimizations.md  what made tpp fast, and what did not
docs/roadmap.md        where tpp is going
tests/spec/            conformance tests, by chapter of the standard
tests/tla/             TLA⁺ models of tpp's algorithms
examples/              a spec and its configuration to try tpp on
bench/                 benchmark specs, compare.py (against TLC), quick.sh (two builds)
CHANGELOG.md           what each release changed; releasing is tagging vX.Y.Z
```

## License

MIT; see [LICENSE](LICENSE). The Windows build ships Cygwin's
`cygwin1.dll`, which is LGPLv3.
