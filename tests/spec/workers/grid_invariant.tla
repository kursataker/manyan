---- MODULE grid_invariant ----
\* The first violating state in a wide level is the one reported.
EXTENDS Naturals
VARIABLES x, y, z
Init == x = 0 /\ y = 0 /\ z = 0
Up(v) == v < 20 /\ v' = v + 1
Next == \/ Up(x) /\ UNCHANGED <<y, z>>
        \/ Up(y) /\ UNCHANGED <<x, z>>
        \/ Up(z) /\ UNCHANGED <<x, y>>
Low == ~(x + y + z = 31 /\ z = 9)
====
INIT Init
NEXT Next
INVARIANT Low
