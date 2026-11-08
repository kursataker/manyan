---- MODULE next_non_function ----
\* A number applied as a function in the next-state relation.
VARIABLE x
Init == x = 0
Next == 1[2] /\ x' = 1
====
INIT Init
NEXT Next
