# tpp manual

tpp checks TLA+ specifications the way TLC does: it reads a module and a
model configuration, explores every reachable state breadth-first, and
prints what TLC 2.19 prints. What tpp accepts and what it must print are
defined by [the tpp TLA+ Standard](standards/tla2.md); this manual covers
how to run it.

## Building

```sh
make            # builds ./tpp from tpp.c and tpp.h; needs a C11 compiler
make release    # builds a faster ./tpp, profile-guided (clang or gcc, python3)
make install    # installs bin/tpp and include/tpp.h under PREFIX (/usr/local)
```

`make release` builds tpp instrumented, runs the conformance suite with it,
and builds it again with the profile; the result is 10 to 20% faster.
`make install` after it installs that build.

tpp is one C file with no dependencies beyond the C library and POSIX
(`fork`, `mmap` for workers).

## Running

```sh
tpp [-config FILE] [-deadlock] [-workers N|auto] [-dump dot FILE] SPEC
tpp -simulate [num=N] [-depth D] [-seed S] [-aril A] [-config FILE] [-deadlock] SPEC
```

`SPEC` is the root module, with or without `.tla`. Modules it extends or
instantiates are read from the same directory; the standard modules
(Naturals, Integers, Sequences, FiniteSets, TLC, Bags, Randomization) are
built in.

| option | meaning |
|---|---|
| `-config FILE` | the model configuration; `SPEC.cfg` by default |
| `-deadlock` | do not report deadlock, as TLC's `-deadlock` |
| `-workers N` | expand states with N processes; `auto` uses one per core |
| `-dump dot FILE` | write the state graph to `FILE.dot` (FILE if it ends in `.dot`) in Graphviz dot format, as TLC does, with one worker; `-dump dot,actionlabels FILE` labels each step with its action |
| `-simulate [num=N]` | check N random behaviors (no limit by default) instead of every state |
| `-depth D` | the length of each random behavior, in states (100) |
| `-seed S`, `-aril A` | choose the random behaviors, as TLC's options do |
| `-version` | print the version and exit |
| `-nowarning` | turn off TLC's warnings, such as that of an EXCEPT outside the domain |
| `-metadir DIR` | accepted for TLC's command lines, and ignored: tpp writes no metadata |

A run prints TLC's report on standard output: the initial states, then
either `Model checking completed. No error has been found.` or the first
error with its trace, then the state counts and the depth of the search.
tpp leaves out TLC's banner, timing and progress lines; everything else
matches TLC's text (Standard §11).

## Model configuration

The configuration file uses TLC's syntax (Standard §9):

```
CONSTANTS N = 3  Procs = {p1, p2}   \* p1, p2 become model values
INIT Init                           \* or SPECIFICATION Spec
NEXT Next
INVARIANT TypeOK Safe
PROPERTY Liveness
CONSTRAINT Bounded
ACTION_CONSTRAINT Steps
SYMMETRY Perms
VIEW Abstract
CHECK_DEADLOCK FALSE
```

`CONSTANT C <- Def` replaces a constant or operator by a definition, and
`CONSTANT D <- [M] Def` or `D = [M] v` a definition `D` of module `M`.
`SPECIFICATION` takes a
formula `Init /\ [][Next]_vars /\ Fairness`; its `WF_` and `SF_`
conjuncts are the fairness assumptions temporal properties are checked
under.

## What is checked

- **Assumptions.** Each `ASSUME` must be true, or the run stops (exit 10).
- **Safety.** Every invariant holds in every reachable state (exit 12 with
  the shortest trace to a violation); every action property `[][A]_v`
  holds on every step.
- **Deadlock.** Each reachable state has a successor, unless deadlock
  checking is off (exit 11).
- **Liveness.** Temporal properties hold on every fair behavior, checked
  after the state graph is complete (exit 13 with a lasso-shaped
  counterexample).
- **Evaluation errors.** An expression TLC cannot evaluate stops the run
  with TLC's message and location (exit 75, or 255 for errors TLC's
  standard-module operators report).

`Print`, `PrintT` and `Assert` behave as in TLC, including the extra
output TLC produces when it reconstructs a trace.

## Simulation

`-simulate` checks random behaviors instead of the whole state graph: from
an initial state drawn at random, it takes a random action's successors,
checks every one of them against the invariants and action properties,
and moves to one at random, until the behavior is `-depth` states long or
stops (Standard §10.8). Temporal properties are checked on each behavior.
With the same `-seed` and `-aril`, tpp draws the same behaviors as TLC
with one worker and prints the same report; `-workers` is ignored. A
seed is drawn at random unless one is given.

## Exit statuses

| status | meaning |
|---:|---|
| 0 | no error |
| 10 | an assumption is false |
| 11 | deadlock |
| 12 | an invariant is violated |
| 13 | a temporal or action property is violated |
| 14 | an `Assert` failed |
| 75 | an evaluation error |
| 76 | an invariant cannot be evaluated in a state the search reached |
| 77 | a temporal property is a tautology |
| 150 | the spec does not parse or fails semantic checks |
| 151 | the configuration names what the spec does not have |
| 255 | another error, such as a configuration that does not parse |

## Workers

With `-workers N`, N processes expand states: tpp forks N - 1 once the
search reaches a level of 256 states or more, and is the last itself. They
share the state store and the fingerprint set, read-only, and expand the
level in batches of 32 states; tpp merges their successors in the order one
process would find them, and expands a batch while it waits for one. A state whose
expansion reports anything (an error, a violation, a deadlock, output from
`Print`) is expanded again by tpp itself, so the output, including which
violation is reported and its trace, is the same for every N. Levels
smaller than 256 states are expanded by tpp alone.

## Library

`tpp.h` declares one entry point:

```c
#include "tpp.h"

TPPOptions o = {.spec = "MySpec", .check_deadlock = 1, .workers = 1};
int status = tpp_check(&o);   /* prints the report; returns an exit status */
```

Compile `tpp.c` with `-DTPP_NO_MAIN` to link it into another program.

## Testing

```sh
make test              # tests/spec with 1 and with 4 workers
make test-spec-oracle  # also runs TLC on every test (downloads tla2tools 1.7.4)
make bless ONLY=name   # writes a test's expected output from TLC
make sanitize          # tests/spec under UBSan (and Guard Malloc on macOS)
make fuzz              # mutated tests/spec models under the sanitizer build
make fuzz-oracle       # random expressions, printed by tpp and by TLC
make fuzz-sim          # random simulations of tests/spec models, by tpp and by TLC
make bench             # times tpp against TLC; writes docs/benchmark.md
```

A test in `tests/spec` is `NAME.tla`: a module, then its configuration
after the line that ends the module. `NAME.out` is TLC's normalized
output and exit status. `tests/spec/tlc` holds models imported from TLC's
own test suite.

`make fuzz` swaps, drops or repeats one to three tokens of a random test's
model (most mutants still parse) and runs the sanitizer build on it, half
the time as a random simulation; it
must end with one of tpp's exit statuses, without a signal or a sanitizer
report. A model that fails is kept as `.obj/crash-SEED-RUN.tla`.
`make fuzz-oracle` writes random constant expressions as `ASSUME
PrintT(e)` and checks that tpp prints what TLC prints, narrowing a
difference to one expression. `make fuzz-sim` simulates random tests'
models with random seeds and depths, with tpp and with TLC. All three take `RUNS=` (1000) and `SEED=` (1);
a run with a seed is repeatable. A case any of them finds becomes a test in
`tests/spec`.

## Limitations

Not supported: the proof language (parsed and skipped), TLC's generation
mode (`-generate`), writing simulated behaviors to files (`-simulate
file=`), checkpoints and distributed checking. In a simulation, output
from `Print` comes in a different order than TLC's, and a temporal
property is checked on the whole graph of each behavior, where TLC may
miss a counterexample of a property such as `P ~> Q`, or report another. Real numbers
(`Reals`, decimals) and `\AA`/`\EE` give TLC's errors, as TLC cannot
check them either. `TLCSet` is accepted and does nothing; `TLCGet`
supports only `"level"`.
