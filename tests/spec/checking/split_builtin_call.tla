---- MODULE split_builtin_call ----
\* A call of a built-in operator is one action, named by it and located at
\* the call.
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x' = x + 1 \/ Assert(x < 2, "big")
Inv == x < 3
====
INIT Init
NEXT Next
INVARIANT Inv
