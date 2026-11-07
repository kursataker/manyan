---- MODULE postcondition ----
\* A simulation checks the postconditions when its behaviors are done.
\* options: -simulate num=3 -depth 5 -seed 1
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Post == TLCGet("diameter") = 99
====
INIT Init
NEXT Next
POSTCONDITION Post
CHECK_DEADLOCK FALSE
