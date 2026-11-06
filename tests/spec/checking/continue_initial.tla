---- MODULE continue_initial ----
\* -continue goes on from an initial state that violates an invariant.
\* options: -continue -workers 4
EXTENDS Naturals
VARIABLE x, y
Init == x \in {0, 5} /\ y = 0
Next == x < 7 /\ x' = x + 1 /\ y' = y + x
Inv == x # 5
Inv2 == y < 12
====
INIT Init
NEXT Next
INVARIANT Inv Inv2
CHECK_DEADLOCK FALSE
