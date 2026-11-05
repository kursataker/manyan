---- MODULE primed_expressions ----
EXTENDS Naturals
VARIABLES x, y
Init == x = 0 /\ y = 0
Next == /\ x' \in 0..2
        /\ y' = x' + x
        /\ x' + y' < 4
        /\ (x' > x) = (y' > y)
====
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
