---- MODULE view_initial ----
\* -view prints an initial state that violates an invariant as the VIEW's
\* value.
\* options: -view
EXTENDS Naturals
VARIABLE x, y
Init == x = 0 /\ y = 0
Next == x < 3 /\ x' = x + 1 /\ y' = y + 2
V == <<x, "v">>
Inv == x > 0
====
INIT Init
NEXT Next
VIEW V
INVARIANT Inv
