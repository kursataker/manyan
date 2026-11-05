---- MODULE grid_deadlock ----
EXTENDS Naturals
VARIABLES x, y, z
Init == x = 0 /\ y = 0 /\ z = 0
Up(v) == v < 20 /\ v' = v + 1
Next == /\ ~(x + y + z = 28 /\ y = 13)
        /\ \/ Up(x) /\ UNCHANGED <<y, z>>
           \/ Up(y) /\ UNCHANGED <<x, z>>
           \/ Up(z) /\ UNCHANGED <<x, y>>
====
INIT Init
NEXT Next
