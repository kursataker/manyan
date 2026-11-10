# Standard modules

TLC computes some operators of its modules in Java instead of from their
TLA⁺ definitions: the operators of its own standard modules and some of
CommunityModules'. tpp computes them in C. This document lists them and
says how one is written.

## Built in

Naturals, Integers, Reals, Sequences, FiniteSets, TLC, Bags,
Randomization, RealTime, Toolbox and TLCExt are built into `tpp.c`: a spec
need not supply them, and one it supplies is read in their place.

## CommunityModules

A module of [CommunityModules](https://github.com/tlaplus/CommunityModules)
is read from the spec's directory, as TLC reads it from its class path. Its
operators come from their definitions, except those below: TLC overrides
them in Java from the jar, and tpp in `tpp-std.c`, giving what that Java
gives, error messages included.

| module | operators |
|---|---|
| CSV | `CSVWrite`, `CSVWriteRecord`, `CSVRead`, `CSVRecords` (a line too short for `CSVRead`'s columns fails in `CSVRead`, where TLC fails when the record is used) |
| Graphs | `SimplePath`, `AreConnectedIn`, `IsStronglyConnected` |
| IOUtils | `IOEnv`, `atoi`, `IOExec`, `IOEnvExec`, `Serialize` (format `TXT`; another format is the definition's) |
| Json | `JsonDeserialize`, `ndJsonDeserialize` |
| SequencesExt | `SetToSeq` |
| Statistics | `ChiSquare` |
| SVG | `SVGElemToString`, `NodeOfRingNetwork` |
| UndirectedGraphs | `ConnectedComponents` |

## Writing one

A native operator is a C function on the values of `tpp.h`. `tpp_define`
binds it to an operator of every module of that name, when the operator's
definition takes as many arguments, as TLC binds its Java by name. The last
argument is the Java method as TLC names it when the operator fails; tpp
prints it in the same place. As TLC, tpp casts each argument to its
parameter's class there, `tlc2.value.impl.StringValue` say, and fails as
Java's cast does on another value.

```c
#include "tpp.h"

/* SequencesExt's SetToSeq(S): S's elements, in TLC's order. */
static TPPValue *set_to_seq(TPPCall *call, TPPValue **a) {
    TPPValue *s = tpp_elements(a[0]);
    if (!s) tpp_fail(call, "Attempted to enumerate a set that is not enumerable:\n%s", tpp_print(a[0]));
    TPPValue *elems[tpp_size(s) + 1];
    for (size_t i = 0; i < tpp_size(s); i++) elems[i] = tpp_element(s, i);
    return tpp_tuple(tpp_size(s), elems);
}

tpp_define("SequencesExt", "SetToSeq", 1, set_to_seq,
           "public static tlc2.value.impl.Value tlc2.overrides.SequencesExt.SetToSeq(tlc2.value.impl.Value)");
```

Arguments arrive evaluated. With arity `TPP_LAZY` the operator takes any
number of them unevaluated, as TLC's evaluation methods do, and evaluates
each it needs with `tpp_arg`; returning `NULL` then leaves the call to the
definition, as `Serialize` does for a format other than `TXT`.

Values are made and read with the functions `tpp.h` declares. A record is a
function from strings (`tpp_record` makes one TLC's Java makes as a
`RecordValue`), a sequence a tuple, and the elements of sets and the keys
of functions come in TLC's order. Values belong to the evaluation that
made them; a native keeps none past its return. An error does not return:
`tpp_fail` for one TLC's Java raises as an `EvalException`, which TLC
prints as it is, and `tpp_throw` for another exception, which TLC prints
under the Java method that threw it. `tpp_format` is Java's
`String.format` on strings, and `tpp_to_string` a value as TLC's
`toString` writes it, on one line.

Each new native gets a test in `tests/spec/stdlib`, its expected output
from TLC with the CommunityModules jar (`make bless`), and a stub of the
module in the test's `.files` directory with the definition TLC overrides.
