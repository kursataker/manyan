# Apalache gaps

What `-symbolic` does not yet do that Apalache 0.63.0 does, measured on
Apalache's own integration tests (`test/tla/cli-integration-tests.md`),
which `tests/apalache_port.py` ports to `tests/symbolic/apalache`.
Manyan aims to give Apalache's verdict: no error, the violated invariant
or property, or deadlock, and the length of the counterexample. It prints
in TLC's format, with TLC's exit statuses.

## `apalache-mc check`

247 checks in all.

| | checks | |
|---|---:|---|
| ported, giving Apalache's verdict | 183 | invariants (state, action, trace), temporal properties, deadlock, `--cinit`, `Gen` |
| ported, refused as Apalache refuses them | 30 | below |
| skipped | 10 | below |
| not ported: options `-symbolic` lacks, or files the test lacks | about 24 | below |

### Skipped

The reason is in `SKIP` of `tests/apalache_port.py`.

- InfDomFun: Apalache reports `f[0] = 42` violated where `f[0]` is 42.
  Its verdict is wrong, so not one to give.
- FunInInfiniteSubset: choosing an element of `[P -> SUBSET Int]`, a
  function to arbitrary sets of integers. Bounding the sets' size would
  miss behaviors.
- SafeMath, SafeMath_2: integers of more than 64 bits.
- TestFunctions: Apalache itself runs out of 4 GB.
- FoldSetInInit: `ApaFoldSet` over `SUBSET (1..10)` takes more than 1 GB.
- Selections: Apalache refuses a disjunct that assigns other variables
  than its siblings, by a syntactic analysis Manyan does not copy.
- Bug593: Apalache refuses a `LET` name that clashes with a variable once
  it flattens the instances.
- Bug2772_2: Apalache refuses to expand a set of functions past its limit.
- ConfigUnsorted: Apalache refuses the cycle the configuration's
  substitutions make among the definitions.

### Options -symbolic lacks

- `--view` (3 checks).
- `--max-error` (3 checks): more than one counterexample.
- `--tuning-options`, `--tuning-options-file` (9 checks): Apalache's
  solver settings; most do not change the verdict.

### Specifications Apalache refuses

-symbolic refuses, as Apalache does: a type error; a missing `Init`,
`Next` or invariant; a recursive operator or function; a set or `<<>>`
whose type is left open; a `Repeat` whose count is no literal; a constant
the configuration gives a value not of its type. These 30 checks expect
Apalache's refusal, which `tests/symbolic_runner.py` compares as
`Refused`, without the message. The four refusals skipped above remain,
so Manyan may check a specification Apalache rejects.

### Differences in what is checked

- A temporal property's counterexample is a lasso. Apalache closes a loop
  only where its tableau's own variables repeat too, so it may find only
  a longer lasso than Manyan; the length is not compared.
- Fairness (`WF`, `SF`) in the specification is refused with temporal
  properties: it needs `ENABLED`, which neither checks symbolically.
- Constraints and action constraints are not supported.

## Other commands

| command | tests | |
|---|---:|---|
| `typecheck` | 75 | `-typecheck`: all give Apalache's outcome (`tests/typecheck/apalache`) |
| `parse` | 17 | Apalache's own parser and its outputs (JSON, TLA+) |
| `simulate` | 2 | random symbolic runs |
| `test`, `server`, `version`, `help` | 6 | |

Only `typecheck` is implemented. On the 62 annotated modules of
tlaplus/Examples, `-typecheck` gives Apalache's outcome on 58. Of the
others, two (APEWD840, EWD998) use TLAPS, a module Manyan lacks. In two
(SPA_Attack, SPA_Attack_New) several `VARIABLE`s lack an annotation:
Apalache names the last in SANY's hash table, Manyan the last declared.
`-symbolic` uses the types for `Gen` and for refusing what Apalache
refuses. It makes a `Gen` only where a call has one type: not in an
operator whose parameters leave it open.

## Output

Manyan prints TLC's format and exit statuses: a temporal property's
violation exits with 13, where Apalache exits with 12 for every
violation. Apalache's counterexample files (`violation*.tla`, ITF JSON)
are not written.
