---- MODULE init_replay ----
EXTENDS Naturals, TLC
VARIABLE x
Init == x \in {1, 2} /\ PrintT(<<"init", x>>)
Next == x' = x
Inv == IF x = 2 THEN 3 ELSE TRUE
====
INIT Init
NEXT Next
INVARIANT Inv
