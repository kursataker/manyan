---- MODULE deadlock ----
\* A random behavior that reaches a deadlock.
\* options: -simulate -depth 20 -seed 3
EXTENDS Naturals
VARIABLE x
Init == x \in {0, 1}
Next == x < 4 /\ (x' = x + 1 \/ x' = x + 2)
====
INIT Init
NEXT Next
