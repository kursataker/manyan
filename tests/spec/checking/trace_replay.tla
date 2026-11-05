---- MODULE trace_replay ----
EXTENDS Naturals, TLC
VARIABLE x
Init == x \in {0, 10} /\ PrintT(<<"init", x>>)
A == PrintT(<<"A", x>>) /\ x' = x + 1
B == PrintT(<<"B", x>>) /\ x' = x + 2
Next == A \/ B
Inv == x < 4
====
INIT Init
NEXT Next
INVARIANT Inv
