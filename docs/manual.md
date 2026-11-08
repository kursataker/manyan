# tpp manual

tpp checks TLA+ specifications the way TLC does: it reads a module and a
model configuration, explores every reachable state breadth-first, and
prints what TLC 1.8.0 prints. What tpp accepts and what it must print are
defined by [the tpp TLA+ Standard](standards/tla2.md); this manual covers
how to run it.

## Building

```sh
make            # builds ./tpp from tpp.c and tpp.h; needs a C11 compiler
make install    # installs bin/tpp and include/tpp.h under PREFIX (/usr/local)
```

`make` builds tpp instrumented, runs the conformance suite with it, and
builds it again with the profile; the result is 10 to 25% faster than a
plain `-O2` build. It needs python3 to run the suite and, with clang,
`llvm-profdata` (Xcode has it); without them, it builds tpp plain.

tpp is one C file with no dependencies beyond the C library and POSIX
(`fork`, `mmap` for workers).

## Running

```sh
tpp [-config FILE] [-deadlock] [-workers N|auto] [-dump dot FILE] SPEC
tpp -simulate [num=N][,file=FILE] [-depth D] [-seed S] [-aril A] [-config FILE] [-deadlock] SPEC
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
| `-simulate file=FILE` | also write each behavior that ends without an error to `FILE_0_N`, a module defining its states `STATE_1`, `STATE_2`, ..., each after a comment naming its action, as TLC does |
| `-depth D` | the length of each random behavior, in steps (100) |
| `-seed S`, `-aril A` | choose the random behaviors, as TLC's options do |
| `-version` | print the version and exit |
| `-nowarning` | turn off TLC's warnings, such as that of an EXCEPT outside the domain |
| `-continue` | go on after a violated invariant or action property, as TLC does: each new state's first violated invariant and each step's violated action property is reported, and the search ends with `No error has been found.`; a simulation ends the behavior and exits 255 at the end; one worker |
| `-view` | print each state an error reports as the value of the configuration's `VIEW`, as TLC does |
| `-userFile FILE` | write the output of `Print` and `PrintT` to FILE instead of standard output |
| `-difftrace` | print each state of a trace after the first as the variables that changed, as TLC does; the last state model checking reports is printed whole |
| `-metadir DIR`, `-fp N`, `-fpmem F`, `-fpbits N`, `-gzip`, `-cleanup` | accepted for TLC's command lines, and ignored: tpp writes no metadata and has its own fingerprints |

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
ALIAS Shown                         \* how a trace's states print
POSTCONDITION Done                  \* checked once the search is done
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
and moves to one in the constraints at random, until the behavior has
taken `-depth` steps or stops (Standard §10.8). Temporal properties are checked on each behavior.
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
| 76 | an invariant is not a state predicate, or cannot be evaluated in a state the search reached |
| 77 | a temporal property is a tautology |
| 150 | the spec does not parse or fails semantic checks |
| 151 | the configuration names what the spec does not have, or a constant invariant that is not `TRUE` |
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
make test-spec-oracle  # also runs TLC on every test (downloads tla2tools 1.8.0)
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

Not supported: the proof language (parsed and skipped); TLC's
`-terse`, `-maxSetSize` (tpp does not bound the sets it builds, where TLC
fails past 10^6 elements), `-dfid`, `-coverage`, `-tool`,
`-generateSpecTE`, the configuration's `_PERIODIC`, `_RL_REWARD` and
`_POSSIBLE`, TLC's standard modules `TLCExt` and `Json`, checkpoints (`-checkpoint`, `-recover`) and
distributed checking. In a simulation
that stops at an error before its last behavior, TLC's worker goes on
with the next one while the behavior is printed, so its `Print` output
may come between the states; tpp stops at the error. There a temporal
property is checked on the whole graph of each behavior, where TLC may
miss a counterexample of a property such as `P ~> Q`, or report another. Real numbers
(`Reals`, decimals) and `\AA`/`\EE` give TLC's errors, as TLC cannot
check them either. `RandomElement`, `RandomSubset` and
`RandomSetOfSubsets` draw as TLC in a simulation, and in the assumptions
and constants when model checking. In `Init` and `Next`, TLC seeds each
state's draws with its fingerprint, which tpp computes differently, so
tpp draws other values there.
