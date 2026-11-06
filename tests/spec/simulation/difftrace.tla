---- MODULE difftrace ----
\* -difftrace in a simulation: the violating state too shows only what
\* changed.
\* options: -difftrace -simulate num=1 -depth 10 -seed 1
EXTENDS Naturals
VARIABLE x, y, z
Init == x = 0 /\ y = 0 /\ z = "a"
A == x < 2 /\ x' = x + 1 /\ UNCHANGED <<y, z>>
B == x = 2 /\ y' = y + 1 /\ z' = "b" /\ UNCHANGED x
Next == A \/ B \/ (y = 1 /\ UNCHANGED <<x, y, z>>)
Inv == y < 1
====
INIT Init
NEXT Next
INVARIANT Inv
