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
| ported, giving Apalache's verdict | 179 | invariants (state, action, trace), temporal properties, deadlock, `--cinit` |
| skipped | 9 | below |
| not ported: options `-symbolic` lacks | about 20 | below |
| Apalache refuses the specification | 37 | below |

### Skipped

The reason is in `SKIP` of `tests/apalache_port.py`.

- InfDomFun: Apalache reports `f[0] = 42` violated where `f[0]` is 42.
  Its verdict is wrong, so not one to give.
- FunInInfiniteSubset: choosing an element of `[P -> SUBSET Int]`, a
  function to arbitrary sets of integers. Bounding the sets' size would
  miss behaviors.
- SafeMath, SafeMath_2: integers of more than 64 bits.
- Bug2973, Bug2973_2, Bug3158: `Gen`, whose bound comes from Apalache's
  types (see `apalache-mc typecheck` below).
- TestFunctions: Apalache itself runs out of 4 GB.
- FoldSetInInit: `ApaFoldSet` over `SUBSET (1..10)` takes more than 1 GB.

### Options -symbolic lacks

- `--view` (3 checks).
- `--max-error` (3 checks): more than one counterexample.
- `--tuning-options`, `--tuning-options-file` (9 checks): Apalache's
  solver settings; most do not change the verdict.

### Specifications Apalache refuses

37 checks expect Apalache to stop with an error: 34 with exit status 255
(an unsupported or ill-formed specification, such as a quantifier over
`Seq(S)`), 2 with 75 and 1 with 120 (a type error). None is ported, so
Manyan may check a specification Apalache rejects.

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
| `typecheck` | 75 | Snowcat, Apalache's type checker: `@type` annotations, inference, records, variants, rows |
| `parse` | 17 | Apalache's own parser and its outputs (JSON, TLA+) |
| `simulate` | 2 | random symbolic runs |
| `test`, `server`, `version`, `help` | 6 | |

None is implemented.

## Output

Manyan prints TLC's format and exit statuses: a temporal property's
violation exits with 13, where Apalache exits with 12 for every
violation. Apalache's counterexample files (`violation*.tla`, ITF JSON)
are not written.
