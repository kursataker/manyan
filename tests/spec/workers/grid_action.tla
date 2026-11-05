---- MODULE grid_action ----
EXTENDS Naturals
VARIABLES x, y, z
vars == <<x, y, z>>
Init == x = 0 /\ y = 0 /\ z = 0
Up(v) == v < 20 /\ v' = v + 1
Next == \/ Up(x) /\ UNCHANGED <<y, z>>
        \/ Up(y) /\ UNCHANGED <<x, z>>
        \/ Up(z) /\ UNCHANGED <<x, y>>
Spec == Init /\ [][Next]_vars
Steady == [][~(x' > x /\ y = 11 /\ z = 16)]_vars
====
SPECIFICATION Spec
PROPERTY Steady
