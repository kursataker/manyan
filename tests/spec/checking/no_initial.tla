---- MODULE no_initial ----
EXTENDS Naturals
VARIABLE x
Init == x \in 1..3 /\ x > 5
Next == UNCHANGED x
====
INIT Init
NEXT Next
