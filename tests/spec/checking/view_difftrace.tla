---- MODULE view_difftrace ----
\* With -difftrace TLC prints a state after another as what changed, the
\* first and the last as the VIEW.
\* options: -view -difftrace
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
