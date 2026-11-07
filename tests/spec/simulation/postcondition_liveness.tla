---- MODULE postcondition_liveness ----
\* After a behavior violates a temporal property the simulator checks the
\* postconditions.
\* options: -simulate num=3 -depth 7 -seed 326
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
