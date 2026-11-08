# Changelog

A release is tagged `vX.Y.Z`, the version `TPP_VERSION` in `tpp.h` names.
Pushing the tag builds, tests and publishes it, with this file's section
for it as its notes.

## Unreleased

Edge cases found running tlaplus/Examples' models against TLC 1.8.0:

- A property or specification is split through any number of instances
  (TestGraphs' `Refinement == MCR!Spec` through `INSTANCE TLCMC`).
- TLC builds its tableaux before the assumptions; a constant in a
  property that fails there is TLC's unexpected exception.
- Fingerprint stack traces have the `INSTANCE` an evaluation enters, and
  the set of `UNION {S}`.
- `TLCGet("-Dk")` and `TLCSet("-Dk", v)` are Java's system properties.
- A module in no file of its own is read from the root module's file, as
  SANY reads a monolithic spec (braf's `Common`).
- An action `\E x \in S : A` whose `S` is empty is one action, as TLC
  splits it, so a random behavior draws the same actions (EWD687a_anim,
  whose leaves have no out-edges); one whose `S` fails is one action
  too, failing in the search rather than before it.
- A call `Op(a)` in the next-state action whose argument is not
  constant, or fails, or of a built-in `Op`, is one action named `Op`
  and located at the call, as TLC splits it; a call of a LET definition
  is split.

Edge cases found running TLC's own test models (`test-model`):

- A tab moves to the column after the next multiple of 8, as SANY counts
  it, for junction alignment and locations.
- A constant operator, and an instance's definition (`I!Op`), are
  operator arguments by their bare names.
- An operator passed as an argument takes its arguments lazily, as a
  definition does.
- A property is split as TLC's `processConfigProps` splits it: through
  LET, calls of definitions with arguments, and `\A x \in S` over a
  constant set; a parameter has its argument's level, so
  `Prop(a) == [](a < 2)` checks `Prop(s)` as an invariant `Prop`
  instead of failing to evaluate `s` without a state.
- A safety-like temporal property, such as `[](x = 2 => [](x >= 2))`,
  checked without fairness, fails as TLC finds it: on a path back to a
  state already expanded, the search stops there with a finite behavior,
  that moment's counts and exit status 12, instead of a lasso at the end
  (Github1161Violated).
- Bags' operators but `SubBag` are TLC's Java methods, with their
  results and errors: `IsABag(<<"a">>)` is `FALSE` instead of failing to
  compare `"a"` with `Nat`, and `CopiesIn(2, 2 :> -1)` is `-1` (BagsTest).
- The configuration's `CONSTANT F(0) = 1  F(1) = 2` defines the operator
  `F` by its values, as TLC does; tpp skipped the arguments and took the
  last value (ConstantOperatorConfiguration). A constant whose value is
  no operator, applied to arguments, is that value, as in TLC.
- `S \subseteq T` is TLC's: `SUBSET A \subseteq SUBSET B` is
  `A \subseteq B`, an interval in an interval is decided by its bounds,
  and an error inside `S` is one TLC cannot fingerprint (EmptySubsetEq).
- `SUBSET S` of 31 elements or more fails as TLC counts it, with each
  SUBSET inside in the fingerprint stack; a successor TLC finds again,
  comparing SUBSETs by their bases, fails again rather than with its
  NullPointerException (FingerprintExceptionInit, FingerprintExceptionNext).
- A temporal `IF` whose guard evaluates with no state is its chosen
  branch, as TLC 1.8 translates it, so a `RECURSIVE` property ends
  instead of overflowing the stack (Github1389).
- A configuration string that a line end or the file's end interrupts is
  JavaCC's lexical error, as TLC reports it (Github1455).
- `TLCGet` is no constant to TLC, as its Java method's level is 1: a
  definition using it is not evaluated at startup, and a substitution
  `A <- d` of one that fails there is the error of a non-constant
  substitution. At startup `TLCGet("level")` is 0 and the search's
  counters are undefined (TLCGetNamedUndefined). `TLCGet("level")'` is
  the next state's level (TLCGetLevel).
- An error in a temporal property on an initial state exits with the
  error's status, such as 151 for an undefined identifier
  (InitialLivenessEvaluationError).
- After a deadlock, a safety-like temporal property violated on the path
  to the deadlocked state is reported too, as TLC adds that state to its
  temporal checks after reporting the deadlock (Github710d).
- An alias's field `_format` is the Java format each other field is
  printed with; a conversion Java cannot apply to a string is its
  exception, ending the trace (PrintTraceException).
- A conjunct `[]S` of the specification, `S` a state predicate, is the
  temporal formula TLC cannot handle; the specification is taken before
  the properties, as TLC takes it (Github525).
- A substitution that makes the subscript of `[][Next]_v` refer to itself
  is TLC's stack overflow, as it collects the subscript's variables even
  with `-nowarning` (CyclicRedefineVars).
- Fairness and properties whose disjunctive normal form TLC cannot count
  in an int, such as 36 `SF`s, are its error before the check (Github742).
- `I!F \in S` with `F == x'`, `(LET y == x' IN y) = 0` and `I!G' = 0`
  assign `x'`, in the next-state action and in `ENABLED`, as TLC 1.8's
  getPrimedVar finds the variable (Github725c, d, e).
- Inside `ENABLED`, a primed variable of an instance that the action
  assigned by name is read back by name (Github725f, g).

- TLCExt: `AssertError`, `TLCCache`, `TLCModelValue`, `TLCGetOrDefault`,
  `AssertEq`, `TLCNoOp` and `TLCGetAndSet` (ConstantContextTLCCache,
  ModuleSemanticsAssume); its operators of traces, fingerprints and
  successors stop tpp, saying so.
- `RandomSubset(k, a..b)` takes its elements without making the interval,
  so it draws from up to 2^31 - 1 numbers; a negative `k` is Java's
  NegativeArraySizeException.
- Init draws on from where the assumptions' draws stopped, as TLC's main
  thread keeps one generator (RandomSubset's trace); RandomElement and
  RandomSubset index an interval among a product's parts without making
  it, so Init stays small.

Performance:

- A state-level substitution of an instance, such as a refinement
  mapping `q <- Map(x)`, is computed once per state, as TLC keeps its
  LazyValue (a refinement check: 2.09 s to 1.55 s).
- `<<x, y>> \in S` probes `S` with `x` and `y`, without making the tuple
  (closure 0.48 s to 0.46 s).

## 0.6.0

- TLC 1.8.0 is the oracle, pinned to the pre-release jar built from
  commit d8fb231, and every test is blessed by it. tpp follows what
  changed:
  - Messages, warnings and exit statuses: a violated temporal property
    is named, configuration errors and SANY's warnings are 1.8.0's, a
    record field named like a definition is linted, and the depth counts
    as 1.8.0 counts it.
  - A string is not a sequence. `SUBSET`, `UNION` and `DOMAIN` have their
    documented precedence. Quantifiers take their bounds sorted, and a
    quantifier over an interval asks its size.
  - Steps are labeled with their action's constant arguments.
  - After a reported error the behavior is rebuilt once more. An error
    rebuilding it is reported as an exception. Fingerprint errors list
    TLC's call stack.
  - `TLCGet("level")` in a state predicate is the state's own, and
    `TLCGet("config")` is a record.
  - `RandomSubset` draws from TLC's linear congruential generator.
  - A simulation names its initial states by Init and checks successors
    as TLC's worker generates them. A violating successor after the first
    is reported, an error in Init is an exception, `TLCSet("exit")` ends
    the run and `TLCGet("config")` is the simulation's.
  - A step through an instance's specification is named `Action`.
  - `[A]_v` and `<<A>>_v` compare `v` first. A temporal formula with an
    action not of the form `<>[]A` or `[]<>A` is an error, and a
    counterexample ending in stuttering warns of missing fairness.
- `ALIAS` prints a trace's states as the alias's value. `POSTCONDITION`s
  are checked once the search is done, and after an error. The
  configuration knows `_PERIODIC`, `_RL_REWARD` and `_POSSIBLE`, and
  rejects them.

## 0.5.1

- `make` builds `tpp` profile-guided, trained on the conformance suite
  (10 to 25% faster); without python3, or with clang but no
  llvm-profdata, it builds plain. `make release` is gone.
- Builds without warnings under GCC 15.
- `examples/DieHard` gives the README a spec to run as written.

## 0.5.0

- Faster set operations: `S \cap T` and `S \ T` keep `S`'s order instead
  of sorting again, sets of atoms compare inline, and `SUBSET S` is
  enumerated in the standard order (partition bench 0.27 → 0.19 s).
- Liveness checking keeps each state's product nodes beside it instead of
  in a hash table over the whole product, and their edges in an arena; it
  no longer leaks them from one property to the next (liveness bench
  0.26 → 0.22 s; 30% less memory on larger models).
- `UNION S` stays lazy, as in TLC, when an element of `S` is not an
  enumerated set: membership no longer enumerates it, and iterating it
  visits each element set in turn, duplicates included, in TLC's order
  (MCDisruptor_MPMC now matches TLC, in 0.6 s; it timed out after 60 s).
- Cartesian products, record sets and function sets are generated in
  order instead of sorted afterwards (GameOfLife 11.9 → 6.1 s).
- Each string has one value, made once, instead of a new one each time a
  literal or a record field is evaluated, and a set of literals such as
  `{"ncs", "e1"}` is computed once (MCBakery 4.5 → 2.6 s).
- Under symmetry, a successor's fingerprint reuses the permuted hashes of
  the values it shares with the state before it (symmetry bench and
  MultiPaxos_MC_small 5% faster).
- A product, `SUBSET`, set of functions or set of records written with
  constants and literals alone is computed once in the search, and keeps
  its elements once enumerated (GameOfLife 6.1 → 3.1 s).
- Membership in a product of intervals checks each part instead of
  searching its elements, and applying a function compares atom keys
  inline (GameOfLife 3.0 → 2.8 s, PaxosCommit 3% faster).
- Computing a constant definition or an assumption keeps only its value:
  what each iteration of a quantifier made is dropped after it, unless a
  value made before took a part of it (MCReachabilityTestAllGraphs: 5 GB
  → 3 MB, 1.29 → 1.0 s). The functions of an enumerated `[D -> R]` share
  their domain.
- Fixed: `ENABLED` computed the spec's constant definitions again instead
  of using their value, so one drawn with `RandomElement` could differ
  inside it (SpanTreeRandom reported a false violation of `Safety`).
- Comparing a set with a set that cannot be enumerated, as sorting
  `UNION {Nat, {-1}}` does, reports TLC's "Attempted to compare the set"
  error.
- Errors name the sets TLC names in its fingerprint stack trace. A value
  computed from a constant definition before the search names none, while
  a definition TLC cannot normalize then is computed again where it is
  used, and named. An infinite set on the right of `\in` in an action
  gives TLC's "not enumerable" report.
- A state holding an infinite set gives TLC's error: the one enumerating
  the set fails with, or that the state contains `Nat`, `STRING` or
  `Seq(S)`, and a filter of it prints as TLC prints it.

## 0.4.0

0.3.0 was never tagged; its changes are released here, with 0.4.0's.

- TLC's `-fp`, `-fpmem`, `-fpbits`, `-gzip` and `-cleanup` are accepted
  and ignored, as `-metadir` is.
- `-view` prints each state an error reports as the `VIEW`'s value, and
  `-userFile FILE` writes `Print`'s output to FILE, as in TLC;
  `TPPOptions.view` and `.user_file` in the C API.
- `-continue` goes on after a violated invariant or action property,
  reporting each as TLC does; `TPPOptions.keep_going` in the C API.
- `-difftrace` prints each state of a trace after the first as the
  variables that changed, as TLC does; `TPPOptions.difftrace` in the C
  API.
- `TLCGet("level")` in a simulation is the behavior's length so far, and
  as a trace is rebuilt, each step's own level; it was 0, and the level
  the search stopped at.
- In a simulation, an action property that fails to evaluate is reported
  with the step, exiting 13, as TLC does; an error TLC raises outside its
  evaluator (a check that is not a boolean, a stack overflow in `Init`)
  is reported plainly; a specification without an initial predicate has
  no state to start from.
- `RandomElement`, `RandomSubset` and `RandomSetOfSubsets` draw as TLC
  2.19 does: from one generator for the assumptions, constants and
  initial states and another for the behaviors of a simulation (both
  seeded with 0, whatever `-seed` is), and from `-seed` when model
  checking. A set of functions, records or tuples draws part by part,
  and a trace is rebuilt drawing again, failing where TLC fails. tpp
  took the first elements.
- A specification with two conjuncts `[][A]_v`, or an action not under
  one, is TLC's configuration error; tpp checked the first, or failed in
  the search.
- `TLCGet("distinct")`, `"queue"`, `"diameter"` and `"duration"` count as
  TLC's search does: a new state is distinct as its invariants are
  checked, and joins the queue after them, an initial state before. A
  simulation has only `"duration"`, the others undefined, as in TLC 2.19.
- An error as TLC rebuilds a trace, such as an `Assert` on a counter that
  has since grown, ends the behavior there without a report, as TLC
  does; after an invariant or action property failed to evaluate, that
  error is the exit status. tpp reported it and exited 255.
- `-simulate file=FILE` writes each behavior that ends without an error to
  `FILE_0_N`, as TLC does; `TPPOptions.trace_file` in the C API.
- An error comparing the parts of a set, record or tuple that an expression
  made names each of them in the fingerprint stack trace, as TLC does;
  sorting a set compares each new element with those before it, so the
  message shows the values in TLC's order. Two of TLC's overridden sets
  (`Nat`, `Any`, `Seq(S)`, ...) compare by the first one's module.
- EXCEPT on a tuple at a key that is not an integer, or on a record at one
  that is not a string, is TLC's error rather than the warning for a
  missing key.
- An interval of 2^31 elements or more is TLC's error ("Size of interval
  value exceeds the maximum representable size") where its size is needed;
  it overflowed and could exhaust memory. A quantifier over an interval
  goes through it without making the set.
- Under a view, a trace shows the states as TLC rebuilds them from their
  fingerprints: the first initial state with the view's value, even one
  outside the constraint, then the first successor of each.
- A module the user supplies with a standard module's name, such as their
  own `FiniteSets.tla`, has TLC's Java operators where the names match, as
  TLC loads its classes by the module's name; its other operators are its
  own.
- An error fingerprinting a successor, such as a set TLC keeps as its
  definition, is TLC's unexpected exception without the behavior; the
  fingerprint stack names the records, tuples and sets that hold it, and
  EXCEPT no longer fingerprints the new value early.
- A value that is not a sequence checked against `Seq(S)` is the Sequences
  module's error, reported without the initial state, as TLC does.
- An instance's variable substituted by a definition that is a variable
  (`m == x`, by default or by `WITH m <- h`) is assigned in `Init`, as
  TLC does; it was an undefined identifier.
- `RandomSetOfSubsets` from Randomization is native, as in TLC, with its
  argument checks; it no longer enumerates `SUBSET SUBSET S`.
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
- A specification whose `Init` the configuration replaces (`Init <-
  MCInit`) reads its initial predicate from the replacement; tpp read the
  replaced definition (tcp/MCtcp from tlaplus/Examples).
- A recursive function, or another function TLC keeps lazy, applied in
  an action or the initial predicate is its body there, as TLC treats it:
  its disjunctions are choices, counted as states generated, and it may
  assign variables. byzpaxos/VoteProof from tlaplus/Examples generated
  27727 states where TLC generates 318877. An argument outside its domain
  there is reported as TLC reports it.
- A property that is a state predicate, or a conjunct of one that is, is
  checked on the initial states as TLC checks it, `Error: Property P is
  violated by the initial state:`; tpp checked it as a temporal property,
  and printed the symmetry warning for it (GermanProtocol and Paxos from
  tlaplus/Examples). A simulation ignores it, as TLC's does.
- `TLCGet` of a key TLC 2.19 does not know, such as `"config"`, is TLC's
  error that it is undefined, and of a value that is no string or
  nonnegative integer TLC's error about its argument (DieHard and
  FiniteMonotonic from tlaplus/Examples).
- A constant in a temporal property is treated as TLC treats it: folded
  through `/\` and `\/`, and, under `[]` or `<>` other than in `[]<>c` or
  `<>[]c`, never evaluated, so its `FALSE` holds there: `<>TRUE \/ <>P`
  is violated where `P` never holds, as TLC reports.
- A constant constraint or action constraint is evaluated as the
  configuration is read, as TLC does: `FALSE` is an error, and `TRUE` is
  dropped, so it draws no warning about constraints in liveness checking.
  That warning, and the one about symmetry, come before the assumptions
  are checked, as in TLC (FiniteMonotonic from tlaplus/Examples). A
  constraint whose value is a numeral, not a boolean, is named by it.
- `TLCGet` fails while TLC starts, so a definition that uses it, such as
  `TLCGet("level") < 3`, is evaluated in each state, not once as a
  constant: as an invariant or constraint it was checked in no state, and
  a search it bounds did not end (TLCGetLevel from TLC's tests). The
  configuration's constraints are read after its properties, as TLC
  reads them.
- Applying a tuple written `<<...>>` outside its domain, in an initial
  state or a step, gives TLC's report that it cannot fingerprint it.
- An error evaluating a temporal property is reported when TLC reports
  it, as it searches: on an initial state, printing that state and going
  on as TLC does, or as an unexpected exception with the counts of the
  state being expanded. tpp reported it after the search, as an error
  of its own.
- `tests/tla/Workers.tla` models one level of the parallel search: its
  batches, claims, give-ups and in-order merge end the level as one
  process does, with the same states, parents and first error.
- `UNCHANGED p`, for an operator's parameter `p`, leaves the variable
  its argument names unchanged; tpp failed, saying the variable was
  undefined (BlockingQueueSplit from lemmy/BlockingQueue).
- With both a `VIEW` and a `SYMMETRY`, a state's view is of the least of
  its permutations, as TLC takes it; tpp ignored the symmetry, finding
  six times the states of Raft from Vanlightly/raft-tlaplus.
- `S \cup T` of a set TLC keeps as its definition, such as a product, and
  a set whose elements do not compare with its own is kept so too, as
  TLC keeps it: `Bad \notin (Data \X {0, 1}) \cup {0, 1}` failed (AB2 from
  Isaac-DeFrain/TLAplusFun). Of two enumerated sets, the error is TLC's.
- Every `S \cup T` TLC keeps as its definition, where `S` or `T` is not
  enumerated as written, such as a set of records, is kept so until it is
  enumerated, as TLC keeps it: a definition never used, such as a union of
  sets of records too large to enumerate, ran out of memory
  (Open-Markets-Initiative/omi-tlaplus-definitions). Enumerated, elements
  that do not compare fail as in TLC; a record compared with another value
  gets TLC's message.
- Tuples, sets, records, `CASE` arms, `LET` definitions, arguments and
  bound names are of any number: a tuple of more than 256 elements was a
  syntax error (a byte capture from omi-tlaplus-definitions), a `CASE` of
  more than 127 arms lost the others, and more than 32 bound names
  overran the parser's stack.
- `S \cup T`, `S \cap T` and `S \ T` of a value that is not a set fail
  only where TLC fails, enumerating the result or looking in it, with
  TLC's messages; tpp failed at once, with a message of its own. Kept as
  their definitions, they are values TLC cannot fingerprint when it
  replays a step or `Init`.
- After an error in a property on an initial state, the invariant is
  checked as TLC checks it replaying `Init`, and its error reported once
  (CBCCasperSpec from Isaac-DeFrain/TLAplusFun).
- A constant definition named as `INIT` or `NEXT` is TLC's error that it
  cannot be a constant, and one that is a conjunct of the specification
  is TLC's value: `TRUE` is dropped, `FALSE` makes the spec trivial, and
  another value cannot be a spec. tpp checked a constant `Init`, finding
  one state with no variables where TLC finds none (IsEven from
  miguelmota/tla-cookbook).
- `TLCSet("exit", TRUE)` stops the search once the state whose expansion
  asked for it is expanded, as TLC stops: the states found are left on
  the queue, and the properties checked on the graph of those expanded.
  It was an error (tlaplus-x/tlx).
- A definition of an instance without parameters that is a constant is
  computed as one, as TLC computes it: in an action it made choices,
  generating more states than TLC.
- A function of strings is printed as a record only if each string is a
  name, as TLC prints it: `("a b" :> 1)`, `("WF_x" :> 1)`.
- A substitution in the configuration for a name the spec does not have
  is TLC's error; tpp ignored it.
- Several constants without a value, or substituted by undefined names,
  are reported in the order TLC takes them, that of SANY's hash table of
  the module's symbols; constants before definitions, unassigned before
  wrongly substituted.
- `RandomSubset(k, S)` of the Randomization module is native, taking k
  elements of a finite set, the first, as `RandomElement` takes one; it
  enumerated `SUBSET S` (SmokeEWD998 from lemmy/ewd998).
- An unknown operator applied to arguments is located at its name, as
  SANY locates it.
- A value with a model value or record field whose name has other than
  letters and digits, such as `start_ts`, is printed on one line, as
  TLC's pretty printer, which cannot parse it, prints it (pingcap/tla-plus).
- Symmetry reduction is 3x faster: a state's fingerprint under every
  permutation is computed in one pass over its values, without building
  the permuted values, and a part no permutation moves is hashed once.
- Fixed undefined behavior and a read past the end of an array, found by
  running the sanitizers over the tlaplus/Examples models.
- A step that leaves several variables unassigned names them all.
- A standard module's operator, such as `\div` or `Assert`, failing on an
  initial state is reported without the state, as TLC reports it; a
  failed `Assert` there exits 75, not 14.
- TLC's checks of a configuration's substitutions: a right-hand side
  that is itself substituted for, sides with different numbers of
  arguments (tpp crashed applying one), a module the specification does
  not have, and an undefined name or missing operator in `<- [M]`.
- Definitions that refer to each other forever, as a substitution such
  as `Init <- ReDefInit` can make them, are TLC's StackOverflowError;
  tpp crashed reading the specification or splitting the next-state
  action into actions.
- A constant view is TLC's error, and "1 state generated, with 0 of them
  distinct" is singular, as TLC prints it.
- After `TLCSet("exit", TRUE)` the temporal properties are checked on the
  states expanded, as TLC checks them; a state found but not expanded
  stuttered forever, a false counterexample.
- `x \in {r \in S : P}` in Init or Next takes the elements of `S` one at
  a time, as TLC does: a state is checked before `P` is evaluated on the
  next element.
- `IsFiniteSet` answers as TLC's `isFinite`: a function is a finite set, a
  value that is no set an error, and a filter over a set that is not
  finite, or an intersection of two such sets, an error.
- A state outside the constraints, or a step outside the action
  constraints, is checked against the invariants and action properties
  each time it is found, as TLC checks it; tpp skipped it, missing a
  violation.
- An error evaluating a constraint or action constraint on a successor
  is TLC's unexpected exception, without the behavior; one evaluating an
  action property is reported as "Evaluating action property P failed."
  An action constraint that is not a boolean is an error, not false.
- `[A]_e` evaluates `e` only when `A` is false, and `<<A>>_e` only when it
  is true, as TLC does; an `A` that is not a boolean has TLC's message.
- TLC's warning that the subscript of the specification's next-state
  relation does not seem to contain a variable, found as TLC finds them:
  through tuples, definitions, their arguments and instances.
- A subscript may be a set, record or function expression, as SANY
  parses it: `[A]_[a |-> x]` was a parse error.
- A part of a property is named as TLC names it: by the definition it
  calls, as `Init2` in `P == Init2 /\ [][A]_v`, or else by its location.
- No initial predicate is TLC's error when there are invariants or
  properties to check, not only variables.
- An unbounded `CHOOSE` evaluated has TLC 2.19's message.

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
