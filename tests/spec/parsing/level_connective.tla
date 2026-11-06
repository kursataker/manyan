---- MODULE level_connective ----
\* A conjunction may not join a temporal formula and an action.
VARIABLE x
Init == x = 0
Next == x' = x
Prop == [](x = 0) /\ Next
====
INIT Init
NEXT Next
