---- MODULE no_deadlock ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 5 /\ x' = x + 1
====
INIT Init NEXT Next
CHECK_DEADLOCK FALSE
