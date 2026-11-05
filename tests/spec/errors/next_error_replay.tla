---- MODULE next_error_replay ----
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0 /\ PrintT(<<"init", x>>)
Next == /\ PrintT(<<"next", x>>)
        /\ \/ x < 2 /\ x' = x + 1
           \/ x = 2 /\ x' = x[1]
====
INIT Init
NEXT Next
