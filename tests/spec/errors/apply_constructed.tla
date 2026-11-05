---- MODULE apply_constructed ----
\* A function made by [x \in S |-> e], not yet in a state.
EXTENDS Naturals, TLC
ASSUME PrintT(LET g == [x \in 1..2 |-> x] IN g[3])
====
