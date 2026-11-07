---- MODULE postcondition_initial ----
\* So too after an initial state violates an invariant.
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Inv == x > 0
Post == TLCGet("distinct") = 3
====
INIT Init
NEXT Next
INVARIANT Inv
POSTCONDITION Post
