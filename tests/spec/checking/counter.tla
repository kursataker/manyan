---- MODULE counter ----
EXTENDS Naturals
VARIABLES x, y
Init == x = 0 /\ y = 0
Next == \/ x < 3 /\ x' = x + 1 /\ UNCHANGED y
        \/ y < 2 /\ y' = y + 1 /\ UNCHANGED x
        \/ x = 3 /\ y = 2 /\ x' = 0 /\ y' = 0
Inv == x + y <= 5
====
INIT Init
NEXT Next
INVARIANT Inv
