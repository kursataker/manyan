---- MODULE invariant_trace ----
EXTENDS Naturals
VARIABLES x, y
Init == x = 0 /\ y = 0
Next == \/ x < 3 /\ x' = x + 1 /\ UNCHANGED y
        \/ y < 2 /\ y' = y + 1 /\ UNCHANGED x
Inv == x + y < 4
====
INIT Init
NEXT Next
INVARIANT Inv
