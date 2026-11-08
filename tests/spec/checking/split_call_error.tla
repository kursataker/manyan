---- MODULE split_call_error ----
\* A call whose constant argument fails as TLC splits the action is one
\* action, named by the definition and located at the call; the argument
\* is evaluated only if it is used.
EXTENDS Naturals
VARIABLE x
Go(p) == x' = x + 1 \/ x' = x + 2
Init == x = 0
Next == Go(CHOOSE n \in {} : TRUE)
Inv == x < 3
====
INIT Init
NEXT Next
INVARIANT Inv
