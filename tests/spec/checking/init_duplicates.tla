---- MODULE init_duplicates ----
EXTENDS Naturals
VARIABLE x
Init == x \in 0..2 \/ x \in 1..3
Next == UNCHANGED x
====
INIT Init
NEXT Next
