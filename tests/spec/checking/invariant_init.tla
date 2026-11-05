---- MODULE invariant_init ----
EXTENDS Naturals
VARIABLE x
Init == x \in 1..5
Next == UNCHANGED x
Small == x < 4
====
INIT Init
NEXT Next
INVARIANT Small
