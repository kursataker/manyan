# Changelog

A release is tagged `vX.Y.Z`, the version `TPP_VERSION` in `tpp.h` names.
Pushing the tag builds, tests and publishes it, with this file's section
for it as its notes.

## 0.3.0

- An invariant with primes or temporal operators is TLC's error (exit
  76), reported before the configuration's other names, as TLC does. A
  call such as `B(Next)` with `B(d) == ENABLED d` is a state predicate.
- TLC's warnings for symmetry: a symmetry set that permutes nothing, and
  symmetry with a temporal property. As in TLC, only the first warning of
  a run says how to turn them off.
- A counterexample whose cycle is one state, returned to by an action, ends
  in stuttering, as TLC prints it.
- A constant invariant or property is evaluated as the configuration is
  read, as TLC does: one that is `TRUE` is dropped, another value is an
  error.
- SANY's level rules for built-in operators: `=`, `\in`, set and tuple
  constructors, `CHOOSE`, `EXCEPT` and the ranges of bounds take at most
  actions, and `/\`, `\/`, `=>`, `<=>` may not join a temporal formula and
  an action. A temporal formula evaluated as a predicate, and action
  composition, give TLC's messages.
- A proof's steps (`<1>1.`) end the statement of a theorem: a module with
  proofs, such as Paxos.tla, failed to parse.
- When extended modules or an instance give a name two definitions, the
  first is used and SANY's warning printed, as SANY does; tpp rejected
  the module, or used an instance's definition.
- Checking a temporal property keeps no garbage from what it evaluates:
  pcal/MacroRealQuicksort from TLC's tests took 2.4 GB, and takes 35 MB.
- Simulation, as TLC: without a next-state action, a behavior deadlocks
  in its initial state; TLC's own errors, such as a temporal formula in a
  constraint, are printed without the "unexpected exception" preamble;
  an error in the constraint of an initial state prints that state; and
  printing the behavior finds the action of each step, as TLC does, so an
  error doing so is reported again.
- TLC's messages for selecting a field of a value that is not a record,
  and for comparing a boolean, a record or `Nat` with a value of another
  kind; a record equals only a function TLC makes a record of.
- An invariant that is not a state predicate, whose level depends on a
  variable outside its temporal operators, gets TLC's note about the bug
  in SANY's level checking.
- `UNCHANGED v`, where an instance substitutes an expression for its
  variable `v`, leaves the variables of that expression unchanged; it was
  an error (locks_auxiliary_vars/LockHS from tlaplus/Examples).
- Applying a tuple written `<<...>>` outside its domain, in an initial
  state or a step, gives TLC's report that it cannot fingerprint it.
- Fixed undefined behavior and a read past the end of an array, found by
  running the sanitizers over the tlaplus/Examples models.

## 0.2.0

- `-dump dot FILE` writes the state graph to `FILE.dot` as TLC does;
  `-dump dot,actionlabels FILE` labels each step with its action. It wrote
  an empty file before.
- `-nowarning` turns off TLC's warnings, and `-metadir DIR` is accepted and
  ignored, so TLC's command lines run unchanged. Liveness checking with a
  state or action constraint prints TLC's warning about it.
- A union of sets TLC keeps as their definitions, such as
  `[a : S] \cup [b : S] \cup [c : S]`, crashed with a stack overflow.
- A primed expression may be applied or have a field selected: `x'[1]`,
  `r'.f`.
- As TLC: a tuple equals a string that spells it, `<<"a", "b">> = "ab"`,
  and no value is in an empty interval, `"a" \in 1..0` being `FALSE`.
- A nested module may not redeclare or redefine a name of a module it is
  in; SANY's warnings for a redeclared constant or variable are printed.
- Simulation checks a temporal property on the graph TLC builds from the
  behavior, so it reports a violation where TLC does, under fairness and
  for properties such as `P ~> Q`.
- `UNION S` is kept as its definition when an element of `S` is infinite,
  so `1 \in {} \ UNION {{-1}, Nat}` is `FALSE`, as in TLC.
- Instances, as TLC treats them: the configuration may name an instance's
  definitions (`SPECIFICATION I!Spec`); an instance's actions and the
  parts of a property are named as TLC names them; a constant operator is
  substituted implicitly; `ENABLED` and `<<A>>_v` compare an instance's
  variables by name.
- A temporal `IF`, a definition given a value in `CONSTANTS`, and an
  operator applied to a primed variable (`Op(x')` assigning `x`) are
  checked.
- The configuration is read as TLC reads it, before the spec: its syntax
  errors are TLC's, with their lines; `INIT`, `NEXT`, `SPECIFICATION`,
  `SYMMETRY`, `VIEW` and `CHECK_DEADLOCK` take one name, once; `ALIAS`
  and `POSTCONDITION`, which TLC 2.19 does not know, are errors; and
  `D = [M] v` gives a definition of module `M` a value.
- A subscript may name an instance's definition: `WF_I!vars(A)`.
- `SYMMETRY` is the group its permutations generate, as TLC closes it,
  and under symmetry a property's actions are checked on the steps
  themselves; both reported wrong state counts or violations before.
- `\A x \in S : A` in an action is the conjunction of `A` for each `x`,
  so a disjunction in it is a choice, counted as TLC counts states
  generated.
- `Any`, `TLCSet` and `TLCGet`; `CONSTANT D <- [M] Op`, which was ignored;
  `Print` and `ToString` of an infinite union or difference fail as in
  TLC.
- Bound variables are enumerated in TLC's order, and errors raised in Java
  by TLC end the run with TLC's message and exit status.
- More benchmarks: liveness, symmetry, partition, recursion, refinement.

Binaries: macOS (arm64, x86_64), Linux (x86_64, arm64; static), Windows
(x86_64, built with Cygwin, `cygwin1.dll` beside `tpp.exe`; it runs on
Windows on Arm too, emulated). In a Cygwin shell, delete the bundled
`cygwin1.dll`: two in one session conflict.

## 0.1.0

The first release: a model checker for TLA⁺ that checks what TLC checks
and prints what TLC 2.19 prints.

- The TLA⁺ language as TLC evaluates it, the standard modules, and TLC's
  model configuration.
- Checking of assumptions, invariants, action properties, deadlock and
  temporal properties under fairness, with TLC's traces and
  counterexamples; `-workers N` for a parallel search with the same output
  as one worker.
- Random simulation (`-simulate`), drawing TLC's behaviors for a seed.
- TLC's error messages and exit statuses.
- A C API (`tpp.h`).

Not yet supported: generation mode, writing simulated behaviors to files,
checkpoints, distributed checking.

Known differences from TLC, found by the fuzzers:

- A set of sets holding an infinite set, such as `{{-1}, Nat}`, is built
  at once, so `1 \in {} \ UNION {{-1}, Nat}` is an error where TLC, which
  never builds it, answers `FALSE`.
- A simulation of `tests/spec/liveness/sf_refine.tla` may report a
  temporal property violated that TLC finds holds.

Binaries: macOS (arm64, x86_64), Linux (x86_64, arm64; static), Windows
(x86_64, built with Cygwin, `cygwin1.dll` beside `tpp.exe`; it runs on
Windows on Arm too, emulated, as Cygwin has no arm64 port). In a Cygwin
shell, delete the bundled `cygwin1.dll`: two in one session conflict.
