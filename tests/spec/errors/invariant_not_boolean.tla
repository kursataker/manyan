---- MODULE invariant_not_boolean ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == UNCHANGED x
Inv == x
====
INIT Init
NEXT Next
INVARIANT Inv
