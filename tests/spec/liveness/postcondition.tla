---- MODULE postcondition ----
\* After a violated temporal property, its counterexample printed, TLC
\* checks the postconditions.
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Prop == <>(x = 5)
Post == TLCGet("distinct") = 3
====
INIT Init
NEXT Next
PROPERTY Prop
POSTCONDITION Post
CHECK_DEADLOCK FALSE
