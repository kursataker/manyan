---- MODULE generate ----
\* -generate builds each successor by random choices, as TLC does: a
\* disjunct, then by a prime step; each \E variable's binding and each
\* value of x' \in S or x' \subseteq S in a random order; until one is kept.
\* options: -generate num=3,file=walk -depth 4 -seed 5
EXTENDS Naturals, TLC
VARIABLES x, y, z, w
ASSUME PrintT(TLCGet("config").mode)
Init == x = 0 /\ y = {} /\ z = <<0, 0>> /\ w = [a |-> 0, b |-> 0]
A == \E i \in 1..5, j \in {7, 8, 9} : x' = i /\ y' = {j} /\ UNCHANGED <<z, w>>
B == \/ x' \in 10..20 /\ UNCHANGED <<y, z, w>>
     \/ y' \subseteq {1, 2, 3} /\ UNCHANGED <<x, z, w>>
     \/ z' \in (1..3) \X (4..6) /\ UNCHANGED <<x, y, w>>
C == \E <<p, q>> \in {<<1, 2>>, <<3, 4>>} : (x' = p \/ x' = q) /\ UNCHANGED <<y, z, w>>
D == w' \in [a : {1, 2}, b : {7, 8, 9}] /\ UNCHANGED <<x, y, z>>
E == \E u, v \in {1, 2, 3} : x' = u * 10 + v /\ (y' = {} \/ y' = {u}) /\ UNCHANGED <<z, w>>
Next == A \/ (B /\ TRUE) \/ C \/ (D /\ TRUE) \/ (E /\ TRUE)
====
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
