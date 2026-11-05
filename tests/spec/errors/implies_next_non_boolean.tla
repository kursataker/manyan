---- MODULE implies_next_non_boolean ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == (x + 1 => x' = 1) /\ x' = 1
====
INIT Init
NEXT Next
