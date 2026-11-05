---- MODULE unassigned_action ----
\* A successor some action leaves incomplete, when there are several.
EXTENDS Naturals
VARIABLES x, y
Init == x = 0 /\ y = 0
Up == x < 2 /\ y' = x + 1 /\ y' = y
Right == y < 2 /\ y' = y + 1 /\ x' = x
Next == Up \/ Right
====
INIT Init
NEXT Next
