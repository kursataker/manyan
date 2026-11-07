---- MODULE postcondition_error ----
\* A postcondition has no state to read a variable from.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Post == x = 3
====
INIT Init
NEXT Next
POSTCONDITION Post
CHECK_DEADLOCK FALSE
