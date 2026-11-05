---- MODULE prime_apply ----
\* A primed expression applied or selected from: x'[1] is (x')[1].
EXTENDS Naturals, Sequences
VARIABLES x, r
Init == x = <<0>> /\ r = [n |-> 0]
Next == /\ x[1] < 2
        /\ x' = <<x[1] + 1>>
        /\ r' = [n |-> x'[1]]
        /\ r'.n = x'[1]
        /\ \A k \in 1..Len(x') : x'[k] > 0
Inv == r.n = x[1]
====
INIT Init
NEXT Next
INVARIANT Inv
CHECK_DEADLOCK FALSE
