# Roadmap

tpp aims to be the TLC people run every day. It should give the same answers
and the same output, start instantly, need nothing installed, and be easy to
embed. It is not a new model checker with its own semantics, a proof checker
(TLAPS) or a symbolic one (Apalache).

The goals come in order. A later goal never costs an earlier one.

## 1. Same as TLC

Every other goal relies on people trusting a result from tpp as much as one
from TLC.

- Run the specs of [tlaplus/Examples](https://github.com/tlaplus/Examples)
  in CI against TLC's cached results (`make test-examples`), and publish how
  many give identical output. Each spec that differs becomes a test in
  `tests/spec` and then a fix. The goal is all of them.
- Close the gaps the [manual](manual.md#limitations) lists:
  - `-coverage`, `-generateSpecTE`, `-dfid`, `-terse` and `-maxSetSize`;
  - checkpoints;
  - the random draws in `Init` and `Next`, which need TLC's state
    fingerprints;
  - the order of `Print` output in a simulation that stops early.
- Follow new TLC releases. For each one, re-pin
  the oracle and re-bless the tests that changed.
- Model more of tpp's algorithms in `tests/tla`, such as the parallel
  search, liveness checking and symmetry, and fuzz what the models cannot
  reach.

## 2. Fast where TLC is slow

- Most specs are small or medium and get checked again after every edit.
  They should finish in milliseconds, fast enough to check on save, in a
  pre-commit hook or in CI.
- Large models should fit in less memory per state. A state set kept on disk
  would allow state spaces larger than RAM.
- Checking should scale with cores, liveness checking included.
- Before any speedup is kept, `docs/benchmark.md` must show it and show no
  regression elsewhere.

## 3. Embeddable

- Make `tpp.h` a stable C API: load a spec, check it, and walk the result
  without parsing printed text.
- Build bindings on that API: Python first, then an editor or language
  server integration.
- Compile tpp to WebAssembly for a browser playground, where anyone can paste
  a spec and check it with nothing installed.

## What stays

- One C file and one header, with no dependencies.
- Every expected output comes from TLC.
- With any number of workers, the output is identical to a one-worker run.
- An improvement over TLC that changes the output, such as a clearer error
  message, is opt-in. The default output stays TLC's.
