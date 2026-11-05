---- MODULE domain ----
EXTENDS Naturals, TLC
f == [x \in 1..3 |-> x]
ASSUME PrintT(f[4])
====
