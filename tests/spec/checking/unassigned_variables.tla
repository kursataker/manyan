---- MODULE unassigned_variables ----
\* A step that leaves several variables unassigned names them all.
EXTENDS Naturals
VARIABLES x, y, z
Init == x = 0 /\ y = 0 /\ z = 0
Next == x' = x + 1
====
INIT Init
NEXT Next
