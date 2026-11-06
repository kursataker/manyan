---- MODULE view_trace ----
\* -view in a simulation prints each state of the behavior as the VIEW's
\* value.
\* options: -view -simulate num=1 -depth 6 -seed 1
EXTENDS Naturals
VARIABLE x, y
Init == x = 0 /\ y = 0
Next == x < 3 /\ x' = x + 1 /\ y' = y + 2
V == <<x, "v">>
Inv == x < 2
====
INIT Init
NEXT Next
VIEW V
INVARIANT Inv
