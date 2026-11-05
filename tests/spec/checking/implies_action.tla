---- MODULE implies_action ----
EXTENDS Naturals
VARIABLES x, y
Init == x = 0 /\ (x = 0 => y \in {1, 2}) /\ (x = 1 => y = 5)
Next == x' = x + 1 /\ (x < 2 => y' = y)
====
INIT Init
NEXT Next
