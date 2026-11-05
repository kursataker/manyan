---- MODULE initial ----
\* An initial state violates the invariant.
\* options: -simulate -depth 5 -seed 1
EXTENDS Naturals
VARIABLE x
Init == x \in 0..3
Next == UNCHANGED x
Inv == x # 2
====
INIT Init
NEXT Next
INVARIANT Inv
