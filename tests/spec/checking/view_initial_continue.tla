---- MODULE view_initial_continue ----
\* With -continue an initial state violating an invariant, as the VIEW, is
\* followed by no blank line.
\* options: -view -continue
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
