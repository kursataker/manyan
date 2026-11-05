---- MODULE init_error_replay ----
EXTENDS Naturals, TLC
VARIABLE x
Init == x \in {1, 2, 3} /\ PrintT(<<"init", x>>) /\ (x < 3 \/ x[1] = 0)
Next == x' = x
====
INIT Init
NEXT Next
