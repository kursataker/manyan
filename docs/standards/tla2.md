# The tpp TLA+ Standard

Version 0.1. The language tpp accepts and the results it must produce.

## Contents

- [1 Scope](#1-scope)
- [2 Conformance](#2-conformance)
- [3 Lexical structure](#3-lexical-structure)
- [4 Modules](#4-modules)
- [5 Declarations and definitions](#5-declarations-and-definitions)
- [6 Expressions](#6-expressions)
- [7 Values](#7-values)
- [8 Standard modules](#8-standard-modules)
- [9 Model configuration](#9-model-configuration)
- [10 Model checking](#10-model-checking)
- [11 Output](#11-output)
- [12 Exclusions](#12-exclusions)

## 1 Scope

This Standard defines TLA+ Version 2 as a model checker must evaluate it:
the syntax of modules, the meaning of the expressions TLC can evaluate,
the model configuration language, the model-checking procedure, and the
observable results. It follows *Specifying Systems* (Lamport, 2002) and the
TLA+ Version 2 language changes. Where those leave a choice open (the order
in which values print, which errors are reported, how many states are
counted), TLC 2.19 (tla2tools 1.7.4) decides.

The proof language (§12) is parsed only as far as needed to skip it.

## 2 Conformance

### 2.1 Oracle

TLC 2.19 is the oracle. A test of this Standard is a module, a model
configuration, and the output TLC produces for them. An implementation
conforms when, for every test, it produces the same normalized output (§11.1)
and the same exit status (§11.3).

### 2.2 Requirements

An implementation shall

1. accept every module and configuration this Standard defines;
2. produce, for a correct model, the result TLC produces: the same verdict,
   the same counts of states, and the same error traces;
3. report an error where TLC reports one, with the same first line of the
   message for the errors listed in §11.3, and the same exit status;
4. check models with one worker deterministically: the same inputs give the
   same output.

### 2.3 Normative language

*Shall* states a requirement. *Unspecified* means TLC's choice is not part of
this Standard and tests must not depend on it.

## 3 Lexical structure

### 3.1 Characters

A module is ASCII text. Lines end with LF or CR LF. A tab counts as one
column. Columns and lines are numbered from 1.

### 3.2 Module boundaries

Text before the first line that holds `----` (four or more dashes) followed
by `MODULE` is ignored. A module begins `---- MODULE Name ----` (any run of
four or more dashes on each side) and ends with a line of four or more `=`
characters. Text after the end of the outermost module is ignored. A module
may contain nested modules with the same delimiters.

### 3.3 Comments and white space

`\*` starts a comment running to the end of the line. `(*` starts a
comment that ends at the matching `*)`; such comments nest. A line of four
or more `-` inside a module is a separator and is ignored.

### 3.4 Identifiers and keywords

An identifier is a sequence of letters, digits, and `_` that contains at
least one letter and is not all digits followed by a letter-free tail; for
example `x`, `x1`, `_x`, `1a` are identifiers and `12` is a number. The
following are reserved and are not identifiers:

```
ASSUME ASSUMPTION AXIOM BOOLEAN CASE CHOOSE CONSTANT CONSTANTS ELSE
ENABLED EXCEPT EXTENDS FALSE IF IN INSTANCE LET LOCAL MODULE OTHER
STRING SUBSET THEN THEOREM TRUE UNCHANGED UNION VARIABLE VARIABLES
WITH WF_ SF_ LAMBDA RECURSIVE LEMMA PROPOSITION COROLLARY
```

`WF_` and `SF_` are prefixes: `WF_vars` is `WF_` followed by the
subscript `vars`.

### 3.5 Numbers

A numeral is a sequence of decimal digits. `\b`, `\o`, and `\h` (or `\B`,
`\O`, `\H`) introduce binary, octal, and hexadecimal numerals. A decimal
`d.d` is accepted by the grammar (§12).

### 3.6 Strings

A string is enclosed in `"`. The escapes are `\"`, `\\`, `\n`, `\t`, `\r`,
and `\f`. A string does not span lines.

### 3.7 Operator symbols

The lexer takes the longest symbol that is a token. The symbols are the
punctuation `( ) [ ] { } << >> , : :: . .. ... ! @ ' == |-> -> <- _`
together with the operators of §6.1. Backslash words (`\in`, `\cup`, ...)
are single tokens; an unknown backslash word is an error.

## 4 Modules

### 4.1 Units

A module is a header, an optional `EXTENDS M1, ..., Mn`, and a sequence of
units: declarations (§5.1), definitions (§5.2), `INSTANCE`s (§4.3),
`ASSUME`s (§4.5), theorems (§12), nested modules, and separators.

### 4.2 EXTENDS

`EXTENDS M` makes every non-`LOCAL` symbol of `M`, and of the modules `M`
extends, a symbol of the extending module. A name defined twice with
different definitions is an error; the same definition reached twice is
one symbol.

### 4.3 INSTANCE

`INSTANCE M WITH p1 <- e1, ..., pn <- en` imports the definitions of `M`
with each constant and variable `pi` of `M` replaced by `ei`. A parameter
not listed is replaced by the symbol of the same name in the instantiating
module, which must exist. `N(x) == INSTANCE M WITH ...` defines a module
instance: `N(a)!Op(b)` is `Op` of the instance with `x` bound to `a`. A
`LOCAL` instance or definition is not exported.

### 4.4 Module resolution

A module named in `EXTENDS` or `INSTANCE` is, in order: a nested module in
scope, a file `Name.tla` in the directory of the root module, or a
standard module (§8).

### 4.5 ASSUME

`ASSUME e` (or `ASSUMPTION`, `AXIOM`, optionally named `ASSUME N == e`)
states that the constant expression `e` is `TRUE`. Each assumption is
evaluated after the constants are bound (§10.1); a false one is an error
(§11.3).

## 5 Declarations and definitions

### 5.1 Declarations

`CONSTANT c1, ..., cn` declares constant parameters, each a name or an
operator shape such as `F(_, _)` or `_ ** _`. `VARIABLE v1, ..., vn`
declares the state variables. The plural spellings are synonyms.

### 5.2 Operator definitions

`Op == e`, `Op(p1, ..., pn) == e`, and the infix, prefix, and postfix forms
`a ++ b == e`, `-. a == e`, `a ^+ == e` define operators. A parameter may
itself be an operator, written `F(_)` or `_ + _`, and an argument for it is
an operator name or a `LAMBDA x, y : e`. Arguments are passed by name: the
argument expression is evaluated, in the context of the call, where the
parameter is used.

### 5.3 Function definitions

`f[x \in S] == e` defines `f` as `CHOOSE f : f = [x \in S |-> e]`: `f` may
appear in `e`, so the function is recursive. Several bound variables
`f[x \in S, y \in T]` define a function on `S \X T`.

### 5.4 Recursive operators

`RECURSIVE Op(_, _)` declares an operator that a later definition in the
same unit list defines; the definition may then refer to itself. Evaluation
that does not terminate is an error (§11.3).

### 5.5 LET

`LET d1 ... dn IN e` makes the definitions `di` (operator, function, or
`RECURSIVE` declarations) visible in later `di` and in `e`.

### 5.6 Scoping

A bound name may not reuse a name already visible in its scope, except that
names introduced by different definitions do not conflict. A name refers to
the innermost binding.

## 6 Expressions

### 6.1 Precedence

Each operator has a precedence range `[lo, hi]`. In `a op1 b op2 c`, the
expression parses as `(a op1 b) op2 c` when `op1`'s range lies above
`op2`'s, as `a op1 (b op2 c)` when below, and is an error when the ranges
overlap, unless `op1` and `op2` are the same left-associative operator.

| Operators | Range | Assoc. |
|---|---|---|
| `=>` | 1–1 | |
| `~>` `<=>` `\equiv` `-+->` | 2–2 | |
| `/\` `\land` `\/` `\lor` | 3–3 | left |
| `~` `\lnot` `\neg` (prefix) | 4–4 | |
| `[]` `<>` `ENABLED` `UNCHANGED` (prefix) | 4–15 | |
| `=` `#` `/=` `<` `>` `\leq` `<=` `=<` `\geq` `>=` `\in` `\notin` `\subseteq` `\subset` `\supseteq` `\supset` `\prec` `\succ` `\preceq` `\succeq` `\sim` `\simeq` `\ll` `\gg` `\asymp` `\approx` `\cong` `\sqsubset` `\sqsubseteq` `\sqsupset` `\sqsupseteq` `\doteq` `\propto` `:=` `::=` `|-` `-|` `|=` `=|` | 5–5 | |
| `\cdot` | 5–14 | left |
| `@@` | 6–6 | left |
| `:>` `<:` | 7–7 | |
| `\` | 8–8 | |
| `\cap` `\intersect` `\cup` `\union` | 8–8 | left |
| `..` `...` | 9–9 | |
| `!!` | 9–13 | |
| `##` `$` `$$` `??` `\uplus` `\sqcap` `\sqcup` | 9–13 | left |
| `\wr` | 9–14 | |
| `SUBSET` `UNION` `DOMAIN` (prefix) | 10–13 | |
| `\X` `\times` | 10–13 | n-ary |
| `+` `++` `\oplus` `(+)` | 10–10 | left |
| `%` | 10–11 | |
| `%%` `\|` `\|\|` | 10–11 | left |
| `-` `--` `\ominus` `(-)` | 11–11 | left |
| `-` (prefix) | 12–12 | |
| `*` `**` `&` `&&` `\o` `\circ` `\odot` `(.)` `\otimes` `(\X)` `\star` `\bigcirc` `\bullet` | 13–13 | left |
| `/` `//` `\div` `\oslash` `(/)` | 13–13 | |
| `^` `^^` | 14–14 | |
| `'` `^+` `^*` `^#` (postfix) | 15–15 | |
| `[ ]` (application) | 16–16 | left |
| `.` (record field) | 17–17 | left |

`a \X b \X c` is the ternary product, not `(a \X b) \X c`.

### 6.2 Junction lists

A list of formulas each preceded by `/\` (or each by `\/`), whose bullets are
aligned in one column, is their conjunction (disjunction). An item extends
until the first token that begins at or left of the bullet's column; a token
there that is the same bullet starts the next item, and any other token ends
the list. A parenthesis, bracket, or brace that opened inside an item must
close inside it.

### 6.3 Logic

`TRUE`, `FALSE`, `BOOLEAN` (the set `{FALSE, TRUE}`). `~`, `/\`, `\/`, `=>`,
`<=>` (and `\equiv`) on Booleans. `/\` and `\/` evaluate left to right and
stop at the first operand that decides the result; so do `=>` and junction
lists. An operand that is not a Boolean is an error.

`\A x \in S : p` and `\E x \in S : p` over a finite `S`; several bounds
`\A x, y \in S, z \in T : p`; tuple binders `\E <<x, y>> \in S \X T : p`.
Unbounded `\A x : p` and `\E x : p` are errors to evaluate.
`CHOOSE x \in S : p` is the first element of `S`, in the order of §7.4, that
satisfies `p`; none is an error. `CHOOSE x : p` is evaluable only as
§10.3 permits.

`=` and `#` (`/=`) compare values (§7.3).

### 6.4 Conditionals

`IF c THEN a ELSE b`. `CASE p1 -> e1 [] ... [] pn -> en [] OTHER -> e`
evaluates the guards in order and yields the arm of the first true one;
no true guard and no `OTHER` is an error.

### 6.5 Sets

Enumeration `{a, b}` and `{}`; `\in`, `\notin`; `\cup`, `\cap`, `\`;
`\subseteq`; `SUBSET S` (all subsets); `UNION S` (union of the elements);
`{x \in S : p}` (filter); `{e : x \in S, y \in T}` (image). Membership may
be decided for the infinite sets `Nat`, `Int`, `STRING`, `Seq(S)`,
`SUBSET S`, `[S -> T]`, `[a : S]`, and filters of them; enumerating an
infinite set is an error.

### 6.6 Functions

`[x \in S |-> e]` and `[x \in S, y \in T |-> e]`; application `f[a]` and
`f[a, b]` (which is `f[<<a, b>>]`); `DOMAIN f`; the function set
`[S -> T]`; `[f EXCEPT ![a] = e, ![b][c] = e2, !.r = e3]`, where `@` in a
replacement is the old value at that path. Applying `f` outside its domain
is an error.

### 6.7 Records

`[a |-> 1, b |-> 2]` is the function on `{"a", "b"}`; `r.a` is `r["a"]`.
`[a : S, b : T]` is the set of such records.

### 6.8 Tuples

`<<a, b, c>>` is the function on `1..3`. `<<>>` is the empty tuple.
`S \X T` is the set of pairs.

### 6.9 Action operators

In an action, `e'` is `e` evaluated in the next state; `UNCHANGED e` is
`e' = e`; `[A]_e` is `A \/ UNCHANGED e`; `<<A>>_e` is `A /\ ~UNCHANGED e`;
`ENABLED A` is true in a state from which `A` has a successor; `A \cdot B`
is composition.

Inside `ENABLED`, as in TLC, a primed variable of an instantiated module
written in `A` itself is not replaced by its substitution: `x' = e` gives
the variable named `x` a value, so with `INSTANCE M WITH x <- z, y <- z`,
`ENABLED (x' = 0 /\ y' = 1)` in `M` is true. An action passed to an
operator argument from outside the instance keeps its substitutions.

### 6.10 Temporal operators

`[]F`, `<>F`, `F ~> G` (`[](F => <>G)`), `WF_e(A)`, `SF_e(A)`, and
`F -+-> G`. Temporal formulas appear only in `SPECIFICATION` and `PROPERTY`
formulas (§10.5), and in definitions they use.

A temporal formula contains no raw action, a semantic error otherwise:
`[]A` and `<>A` of an action `A` must be `[][A]_e` and `<><<A>>_e`; the
operands of `~>` and `-+->` are not actions; and a quantifier over a
temporal formula has no action-level bound.

## 7 Values

### 7.1 Kinds

A value is a Boolean, an integer, a string, a model value (§9.2), a finite
or infinite set, or a function. Tuples, sequences, and records are
functions: `<<1, 2>> = [i \in 1..2 |-> i]` and
`[a |-> 1] = [x \in {"a"} |-> 1]`.

### 7.2 Integers

Integers are the signed 32-bit range of TLC. A result outside it is an
error (§11.3). `\div` and `%` are floor division and its non-negative
remainder; a zero divisor is an error.

### 7.3 Equality

Values of different kinds are unequal only when one is a model value or
when both are sets or both are functions; otherwise comparing them (for
example `1 = "a"`) is an error. Two sets are equal when they have the same
elements; two functions when they have the same domain and values.

### 7.4 Order

Every set prints and enumerates its elements in the standard order:
Booleans `FALSE` before `TRUE`; integers by value; strings in order of
first appearance (§7.5); model values by declaration order; tuples and
functions by domain size, then element by element; sets by size, then
element by element; `Nat` before `Int`, which equal only themselves.
Elements of different kinds are ordered by kind.

### 7.5 String order

Strings are ordered by when tpp first meets them, in this order: the
built-in names of the language; the identifiers and numerals of the root
module and then of each module it uses, in the order those modules are
read; the string literals of every module, dependencies first; the
configuration; and finally strings made during checking. Record fields are
strings, so records print their fields in this order too.

### 7.6 Printing

A value prints as an expression that denotes it: `TRUE`, `-3`, `"a\"b"`, a
model value's name, `{1, 2}`, `<<1, 2>>`, `[a |-> 1, b |-> 2]`, and a
function on another domain as `(1 :> "a" @@ 2 :> "b")`. A function whose
domain is `1..n` prints as a tuple and one whose domain is a non-empty set
of strings prints as a record. A printed value longer than the line width
(80) is broken as TLC's pretty printer breaks it: each element of a long
set, tuple, or record on its own line, aligned under the first.

## 8 Standard modules

The standard modules are those of tla2tools, with the operators below
evaluated natively.

- **Naturals**: `Nat`, `+`, `-`, `*`, `^`, `<`, `>`, `\leq`, `\geq`, `%`,
  `\div`, `..`.
- **Integers**: Naturals and `Int`, prefix `-`.
- **Sequences**: `Seq`, `Len`, `\o`, `Append`, `Head`, `Tail`, `SubSeq`,
  `SelectSeq`.
- **FiniteSets**: `IsFiniteSet`, `Cardinality`.
- **Bags**: `IsABag`, `BagToSet`, `SetToBag`, `BagIn`, `EmptyBag`, `(+)`,
  `(-)`, `BagUnion`, `\sqsubseteq`, `SubBag`, `BagOfAll`, `BagCardinality`,
  `CopiesIn`.
- **TLC**: `Print`, `PrintT`, `Assert`, `JavaTime`, `TLCGet`, `TLCSet`,
  `:>`, `@@`, `Permutations`, `SortSeq`, `RandomElement`, `Any`,
  `ToString`, `TLCEval`.

`Print(out, val)` writes `out` and `val` and yields `val`; `PrintT(out)`
writes `out` and yields `TRUE`; `Assert(c, out)` is `TRUE` when `c` is and
otherwise an error (§11.3).

## 9 Model configuration

### 9.1 Syntax

A configuration is a sequence of statements; `\*` and `(* *)` comments as
in §3.3. The keywords, each followed by one or more names (or, for
`CONSTANT`, assignments):

| Statement | Meaning |
|---|---|
| `CONSTANT`, `CONSTANTS` | `c = v` binds constant `c` to the value `v`; `c <- Op` replaces `c` by the definition `Op`; `c = c` binds `c` to the model value `c` |
| `INIT`, `NEXT` | the initial predicate and next-state action |
| `SPECIFICATION` | a formula `Init /\ [][Next]_v /\ Fairness` |
| `INVARIANT`, `INVARIANTS` | state predicates checked in every reachable state |
| `PROPERTY`, `PROPERTIES` | temporal formulas the specification must imply |
| `CONSTRAINT`, `CONSTRAINTS` | state predicates bounding the explored states |
| `ACTION_CONSTRAINT`, `ACTION_CONSTRAINTS` | actions bounding the explored transitions |
| `SYMMETRY` | a set of permutations of model values |
| `VIEW` | the expression whose value identifies states |
| `CHECK_DEADLOCK` | `TRUE` (the default) or `FALSE` |

### 9.2 Values

A value in a configuration is a number, a string, `TRUE`, `FALSE`, a
model value (any other name), or a set `{v1, ..., vn}` of values. A model
value equals only itself.

## 10 Model checking

### 10.1 Constants

Each declared constant shall be bound by the configuration. Then the
assumptions are evaluated (§4.5).

### 10.2 Initial states

The initial predicate is evaluated as a generator: a conjunction is
evaluated left to right; a disjunction, an `\E`, an `IF`, or a `CASE` tries
each alternative; a conjunct `v = e` with `v` unassigned assigns `v`, and
`v \in S` with `v` unassigned assigns each element of `S` in turn; any
other formula must be `TRUE` for the alternative to continue. A state in
which some variable is unassigned is an error. Each state produced, once
the state constraints admit it, is an initial state.

### 10.3 Successor states

The next-state action is evaluated in a state `s` as the initial predicate
is, with primed variables `v'` in the place of variables, and unprimed
variables read from `s`. An action is split at its disjunctions, `\E`
quantifiers, and the definitions it refers to; each part that produces a
state is a transition. `UNCHANGED v` assigns `v'` the value of `v`. A
successor in which some variable is unassigned is an error.

### 10.4 Search

States are explored breadth first from the initial states, in the order
they are produced. A state is new when no explored state has the same
fingerprint: its value, or the value of the `VIEW` if one is given, up to
the `SYMMETRY` permutations. Each new state that satisfies the state
constraints, reached by a transition that satisfies the action constraints,
is checked against the invariants and queued. A reachable state with no
successor is a deadlock unless `CHECK_DEADLOCK` is `FALSE`.

The *states generated* count is the number of initial states produced plus
the number of transitions taken, duplicates included; *distinct states* is
the number of new states; the *depth* is the length of the longest shortest
path from an initial state, counted in states.

### 10.5 Properties

A `PROPERTY` holds when every behavior of the specification satisfies it.
A behavior is an infinite sequence of states, each transition of which is a
step of the next-state action or a stuttering step, that satisfies the
specification's fairness conditions. A property of the form `[]P` with `P`
a state predicate is checked as an invariant; `[][A]_v` is checked on
every transition; other properties are checked over the state graph after
the search (liveness checking).

### 10.6 Fairness

`WF_v(A)`: a behavior in which `<<A>>_v` is eventually always enabled is
fair only if it takes infinitely many `<<A>>_v` steps. `SF_v(A)`: if
`<<A>>_v` is enabled infinitely often then infinitely many such steps.

### 10.7 Counterexamples

A trace is the shortest path from an initial state to the bad state. A
liveness counterexample is a path to a cycle (a lasso) on which the
property fails and the fairness conditions hold.

When several lassos qualify, TLC's choice is not specified (it can differ
between runs), so a conforming test admits exactly one.

## 11 Output

### 11.1 Normalized output

Normalized output is the standard output with these lines removed: the
version and start-up banner, `Parsing file`, `Semantic processing`,
`Starting...`, `Computing initial states...`, `Computed N initial
states...`, `Progress(...)`, `Checking temporal properties for`,
`Finished checking temporal properties`, `Implied-temporal checking`,
`Finished in`, the average outdegree, the fingerprint-collision report,
and the block that begins `Error: The error occurred when TLC was
evaluating the nested expressions`. Timestamps are removed from `Finished
computing initial states`, and runs of blank lines are one blank line.
When parsing or semantic analysis fails, the normalized output is the
single line `Error: Parsing or semantic analysis failed.`; the diagnostics
before it are unspecified. The exit status follows as a final line
`exit N`.

### 11.2 Results

Output printed by `Print` and `PrintT` while assumptions are checked comes
first. Then

```
Finished computing initial states: N distinct states generated.
```

and, for a correct model,

```
Model checking completed. No error has been found.
N states generated, M distinct states found, 0 states left on queue.
The depth of the complete state graph search is D.
```

A violation prints its error (§11.3), then, if there is a trace,
`Error: The behavior up to this point is:` and the trace, then the counts.
Each state of a trace is `State i: <Action line l1, col c1 to line l2, col
c2 of module M>` (`<Initial predicate>` for the first) followed by the
state as a junction list `/\ v = value` of its variables in TLC's order
(below), a single variable without the bullet, and a blank line. The action
is named by the definition the step's disjunct belongs to, and located by
that disjunct.

TLC's order of the variables is the reverse of the order in which SANY's
context of the root module, a `java.util.Hashtable`, enumerates them. The
table holds the names of SANY's 72 built-in operators, put in reverse
order of their definition, then the module's symbols in the order SANY
meets them: those of each extended module that are not LOCAL, in that
module's order; then, in the module's text, each declared or defined name,
the name of a RECURSIVE operator where it is declared, the name of a named
theorem or assumption, and the name of a nested module. An instance puts
the definitions of the instantiated module, then its named theorems and
assumptions, as that module's own table enumerates them; a named instance
`I` puts each as `I!Op`, its module instances too, then `I`. A name's hash
is Java's `String.hashCode`; the table starts with 11 buckets, puts a new
entry at the head of its bucket, and when it holds three quarters of its
capacity it grows to twice plus one, moving entries from the last bucket
to the first, each to the head of its new bucket. It enumerates the
buckets from the last to the first, each from its head.

A liveness counterexample is introduced by `Error: Temporal properties were
violated.` and `Error: The following behavior constitutes a
counter-example:`; it ends with `Back to state i: <Action ...>` when it
loops back to state `i`, or `State n: Stuttering` when it stutters forever.

### 11.3 Errors

| Situation | First line | Exit |
|---|---|---|
| assumption false | `Error: Assumption line l, col c to line l2, col c2 of module M is false.` | 10 |
| assumption fails to evaluate | `Error: Evaluating assumption line ... of module M failed.` | 10 |
| deadlock | `Error: Deadlock reached.` | 11 |
| invariant fails in an initial state | `Error: Invariant Inv is violated by the initial state:` | 12 |
| invariant fails | `Error: Invariant Inv is violated.` | 12 |
| action property | `Error: Action property P is violated.` | 13 |
| temporal property | `Error: Temporal properties were violated.` | 13 |
| `Assert` | `Error: The first argument of Assert evaluated to FALSE; the second argument was:` | 14 |
| successor not specified | `Error: Successor state is not completely specified by the next-state action. The following variable is not assigned: v.`; with several actions, `by action A of the next-state relation` | 75 |
| invariant not Boolean | `Error: TLC expected a boolean value, but did not find one. ...` | 75 |
| error evaluating an invariant in an initial state | the evaluation message, then `While working on the initial state:` and the state | 75 |
| error evaluating an invariant in a new state | `Error: Evaluating invariant Inv failed.`, the evaluation message, and the trace to the new state | 76 |
| other error evaluating an action | the evaluation message (below), with the trace to the state | 255 |
| constant not assigned | `Error: The constant parameter K is not assigned a value by the configuration file.` | 151 |
| configuration names an undefined operator | `Error: The initial predicate Op specified in the configuration file` | 151 |
| parse or semantic error | `Error: Parsing or semantic analysis failed.` | 150 |

The evaluation messages, as the line after `Error: Evaluating assumption`
or as the first line of an action error:

| Situation | Message |
|---|---|
| `a \div 0`, `a % 0` | `The second argument of \div is 0.` |
| `0 ^ 0` | `0^0 is undefined.` |
| result outside 32 bits | `Overflow when computing a+b` (the operator and operands) |
| `f[a]` outside `DOMAIN f` | `Attempted to apply function:` / `f` / `to argument a, which is not in the domain of the function.` |
| `t[i]` for a tuple `t` | `Attempted to apply tuple` / `t` / `to integer i which is out of domain.` |
| `r["f"]` for a record `r` | `Attempted to apply the record` / `r` / `to nonexistent record field f.` |
| `r.f` for a record `r` | `Attempted to select nonexistent field "f" from the record` / `r` / location |
| `f[a]` for `f` made by `[x \in S \|-> e]`, not yet in a state or a constant | `In applying the function` / `f,` / `the first argument is:` / `a` / `which is not in its domain.` |
| `CHOOSE` with no witness | `Attempted to compute the value of an expression of form` / `CHOOSE x \in S: P, but no element of S satisfied P.` |
| `CASE` with no true guard | `Attempted to evaluate a CASE with no conditions true.` |
| `1 = "a"` | `Attempted to check equality of integer 1 with non-integer:` / `"a"` |
| `<<>> = {}` | `Attempted to check equality of the function << >> with the value:` / `{}` |
| a non-Boolean `/\` operand | `Attempted to evaluate an expression of form P /\ Q when P was` / `an integer.` |
| quantifier over `Nat` | `TLC encountered a non-enumerable quantifier bound` / `Nat.` |

A message that names an expression is followed by its location, `line l,
col c to line l2, col c2 of module M`.

## 12 Exclusions

Not covered: the proof language (`THEOREM`, `PROOF`, steps; parsed and
ignored), real numbers, `\AA` and `\EE`, labels, `-simulate`, `-generate`,
checkpointing, multiple workers' scheduling, distributed TLC, and the
TLA+ Toolbox outputs (`-tool`).
