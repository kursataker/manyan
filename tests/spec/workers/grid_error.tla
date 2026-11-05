---- MODULE grid_error ----
EXTENDS Naturals
VARIABLES x, y, z
Init == x = 0 /\ y = 0 /\ z = 0
Up(v) == v < 20 /\ v' = v + 1
Next == \/ Up(x) /\ y' = y \div (IF x + y + z = 29 /\ z = 9 THEN 0 ELSE 1) /\ UNCHANGED z
        \/ Up(y) /\ UNCHANGED <<x, z>>
        \/ Up(z) /\ UNCHANGED <<x, y>>
====
INIT Init
NEXT Next
