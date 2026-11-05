---- MODULE action_names ----
EXTENDS Naturals
VARIABLE n
Init == n = 0
Up == n' = n + 1
Double == n > 0 /\ n' = 2 * n
Next == Up \/ Double
Inv == n < 6
====
INIT Init
NEXT Next
INVARIANT Inv
