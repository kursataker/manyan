---- MODULE stuttering ----
\* Steps that do not change the state are left out of the behavior.
\* options: -simulate -depth 30 -seed 11
EXTENDS Naturals
VARIABLE x
Init == x = 0
Stay == UNCHANGED x
Step == x' = x + 1
Next == Stay \/ Step
Inv == x < 3
====
INIT Init
NEXT Next
INVARIANT Inv
