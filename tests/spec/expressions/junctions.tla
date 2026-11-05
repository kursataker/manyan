---- MODULE junctions ----
EXTENDS Naturals, TLC
A == /\ 1 = 1
     /\ \/ 2 = 3
        \/ 4 = 4
     /\ TRUE
B == \/ /\ FALSE
        /\ TRUE
     \/ /\ TRUE
        /\ 1 + 1 = 2
C == /\ TRUE
     /\ FALSE
   \/ TRUE
D(x) == /\ x > 1
        /\ x < 5
ASSUME PrintT(<<A, B, C, D(3), D(7)>>)
====
