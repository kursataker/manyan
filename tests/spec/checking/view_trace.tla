---- MODULE view_trace ----
\* -view prints each state of a trace as the VIEW's value, with no blank
\* line after it, as TLC does.
\* options: -view
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
