---- MODULE implies_init_non_boolean ----
EXTENDS Naturals
VARIABLE x
Init == (1 => x = 1) /\ x = 1
Next == UNCHANGED x
====
INIT Init
NEXT Next
