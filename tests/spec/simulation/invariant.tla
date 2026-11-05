---- MODULE invariant ----
\* A random behavior that violates an invariant: TLC's, for the seed.
\* options: -simulate -depth 10 -seed 1
EXTENDS Naturals
VARIABLES x, y
Init == x \in 0..2 /\ y = 0
Up == x' = x + 1 /\ UNCHANGED y
Flip == y' = 1 - y /\ UNCHANGED x
Next == Up \/ Flip
Inv == x < 6
====
INIT Init
NEXT Next
INVARIANT Inv
