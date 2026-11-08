---- MODULE empty_exists ----
\* An action \E q \in {} : A is one action, which a random behavior draws.
\* options: -simulate -depth 10 -seed 7
EXTENDS Naturals
VARIABLE x
Out(p) == IF p = 1 THEN {} ELSE {p, p + 10}
Go(p) == \E q \in Out(p) : x' = x + q
Init == x = 0
Next == \E p \in 1..3 : Go(p) \/ x' = x + 1
Inv == x < 30
====
INIT Init
NEXT Next
INVARIANT Inv
