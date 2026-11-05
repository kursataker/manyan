---- MODULE specification ----
EXTENDS Naturals
VARIABLES x, y
vars == <<x, y>>
Init == x = 0 /\ y = 0
Next == \/ x < 2 /\ x' = x + 1 /\ y' = y
        \/ y < 2 /\ y' = y + 1 /\ x' = x
Spec == Init /\ [][Next]_vars
TypeOK == x \in 0..2 /\ y \in 0..2
====
SPECIFICATION Spec
INVARIANT TypeOK
