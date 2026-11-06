---- MODULE difftrace ----
\* -difftrace in a counterexample: every state after the first shows
\* what changed, the cycle's too.
\* options: -difftrace
EXTENDS Naturals
VARIABLE x, y, z
Init == x = 0 /\ y = 0 /\ z = "a"
A == x < 2 /\ x' = x + 1 /\ UNCHANGED <<y, z>>
B == x = 2 /\ y < 2 /\ y' = y + 1 /\ z' = "b" /\ UNCHANGED x
C == y = 2 /\ y' = 1 /\ UNCHANGED <<x, z>>
Next == A \/ B \/ C
Spec == Init /\ [][Next]_<<x, y, z>> /\ WF_<<x, y, z>>(Next)
Live == <>(x = 9)
====
SPECIFICATION Spec
PROPERTY Live
