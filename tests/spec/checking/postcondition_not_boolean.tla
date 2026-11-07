---- MODULE postcondition_not_boolean ----
\* A postcondition must be a Boolean.
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Post == TLCGet("distinct")
====
INIT Init
NEXT Next
POSTCONDITION Post
CHECK_DEADLOCK FALSE
