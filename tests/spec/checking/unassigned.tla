---- MODULE unassigned ----
VARIABLES x, y
Init == x = 1 /\ y = 1
Next == x' = 2
====
INIT Init
NEXT Next
