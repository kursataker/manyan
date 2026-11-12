# Apalache gaps

What `-symbolic` does not yet do that Apalache 0.63.0 does, measured on
Apalache's own integration tests (`test/tla/cli-integration-tests.md`),
which `tests/apalache_port.py` ports to `tests/symbolic/apalache`.
Manyan aims to give Apalache's verdict: no error, the violated invariant
or property, or deadlock, and the length of the counterexample. It prints
in TLC's format, with TLC's exit statuses.

## `apalache-mc check`

245 checks in all.

| | checks | |
|---|---:|---|
| ported, giving Apalache's verdict | 190 | invariants (state, action, trace), temporal properties, deadlock, `--cinit`, `Gen`, `--max-error` with `--view` |
| ported, refused as Apalache refuses them | 32 | below |
| skipped | 9 | below |
| not ported | 14 | 12 repeat a check ported but for options that do not change the outcome, such as `--out-dir` and `--tuning-options`; 2 check Quint's JSON, not TLA+ |

### Skipped

The reason is in `SKIP` of `tests/apalache_port.py`.

- InfDomFun: Apalache reports `f[0] = 42` violated where `f[0]` is 42.
  Its verdict is wrong, so not one to give.
- FunInInfiniteSubset: choosing an element of `[P -> SUBSET Int]`, a
  function to arbitrary sets of integers. Bounding the sets' size would
  miss behaviors.
- SafeMath, SafeMath_2: integers of more than 64 bits. Not planned: the
  evaluator every counterexample is replayed through has 64-bit integers
  (TLC 32-bit), and bignums there would slow every model checked.
- TestFunctions: Apalache itself runs out of 4 GB.
- FoldSetInInit: `ApaFoldSet` over `SUBSET (1..10)` takes more than 1 GB.
- Selections: Apalache refuses a disjunct that assigns other variables
  than its siblings, by a syntactic analysis Manyan does not copy.
- Bug593: Apalache refuses a `LET` name that clashes with a variable once
  it flattens the instances.
- ConfigUnsorted: Apalache refuses the cycle the configuration's
  substitutions make among the definitions.

### Specifications Apalache refuses

`-symbolic` refuses, as Apalache does: a type error; a missing `Init`,
`Next` or invariant; a recursive operator or function; a set or `<<>>`
whose type is left open; a `Repeat` whose count is no literal; a constant
the configuration gives a value not of its type; `-max-error` without a
`VIEW`; a set of sets of functions, `{[S -> T] : x \in U}`, which it
would expand. These 32 checks expect Apalache's refusal, which
`tests/symbolic_runner.py` compares as `Refused`, without the message. The three refusals skipped above remain,
so Manyan may check a specification Apalache rejects.

### Differences in what is checked

- A temporal property's counterexample is a lasso. Apalache closes a loop
  only where its tableau's own variables repeat too, so it may find only
  a longer lasso than Manyan; the length is not compared.
- Apalache has no fairness: `WF` and `SF` are left for the user to write
  out, and `ENABLED` too. With fairness in a specification whose
  properties are checked, Manyan gives TLC's verdict: a fair lasso, which may also stutter forever,
  once no invariant is violated and no state deadlocks; a deadlock is a
  state without a successor, where Apalache's is one no behavior goes
  on from. `ENABLED`, in an invariant, a property or an action, is TLC's too.
- Apalache 0.63.0 reads `CONSTRAINT` and `ACTION_CONSTRAINT` and then
  ignores them: with `CONSTRAINT x < 2` it reports `x < 3` violated, a
  state TLC never reaches. Manyan gives TLC's verdict: a state outside
  the constraints, or reached by a step outside the action constraints,
  has its invariants checked but no successors, and is no deadlock.

## Other commands

| command | tests | |
|---|---:|---|
| `typecheck` | 75 | `-typecheck`: all give Apalache's outcome (`tests/typecheck/apalache`) |
| `parse` | 17 | Apalache's own parser and its outputs (JSON, TLA+) |
| `simulate` | 2 | random symbolic runs |
| `test`, `server`, `version`, `help` | 6 | |

Only `typecheck` is implemented, by decision:

- `parse` writes Apalache's intermediate forms, JSON and TLA+, which are
  its own; Manyan reports a parse error as SANY does.
- `simulate` checks random runs to a length; `-symbolic` checks every
  run to it, so its verdict holds for any run `simulate` draws.
- `test` checks that one action, from a state predicate, gives states
  that satisfy another: `-symbolic -length 1` with them as `INIT`,
  `NEXT` and `INVARIANT` does the same.
- `server`, `version` and `help` are Apalache's own interface.

On the 62 annotated modules of tlaplus/Examples, `-typecheck` gives
Apalache's outcome on all, with TLAPS's library beside the two that
extend it (APEWD840, EWD998). Where declarations lack annotations, it
names the one Apalache names: a variable before a constant, the last in
the order of SANY's table of the module's names.
`-symbolic` uses the types for `Gen` and for refusing what Apalache
refuses. A `Gen` in an operator whose parameters leave its type open has
the type each call instantiates.

## Output

Manyan prints TLC's format and exit statuses: a deadlock exits with 11
and a temporal property's violation with 13, where Apalache exits with
12 for every violation. With `-run-dir`, Manyan writes the counterexamples in ITF as
Apalache does. By decision it does not write Apalache's `violation*.tla`
and `violation*.json`, the same trace again in Apalache's own TLA+ and
JSON forms, nor Apalache's own variables in a lasso, those of its
tableau, which ends with ITF's `loop` instead. `vars` is in declaration
order, where Apalache's is its own.
