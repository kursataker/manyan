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
| IOUtils | `IOEnv`, `atoi`, `IOExec`, `IOEnvExec`, `Serialize` (format `TXT`; another format is the definition's) |
| SequencesExt | `SetToSeq` |
| SVG | `SVGElemToString` |
| UndirectedGraphs | `ConnectedComponents` |

## Writing one

A native operator is a C function on the values of `tpp.h`. `tpp_define`
binds it to an operator of every module of that name, when the operator's
definition takes as many arguments, as TLC binds its Java by name. The last
argument is the Java method as TLC names it when the operator fails; tpp
prints it in the same place.

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
function from strings, a sequence a tuple, and the elements of sets and the
keys of functions come in TLC's order. Values belong to the evaluation that
made them; a native keeps none past its return. `tpp_fail` reports an
error as TLC reports one in its Java, and does not return.

Each new native gets a test in `tests/spec/stdlib`, its expected output
from TLC with the CommunityModules jar (`make bless`), and a stub of the
module in the test's `.files` directory with the definition TLC overrides.
