---- MODULE split_call_variable ----
\* A call with an argument that is not constant is one action, named by
\* the definition and located at the call, as TLC splits the action.
EXTENDS Naturals
VARIABLE x
Go(p) == x' = p + 1 \/ x' = p + 2
Init == x = 0
Next == Go(x)
Inv == x < 3
====
INIT Init
NEXT Next
INVARIANT Inv
