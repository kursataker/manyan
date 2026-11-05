---- MODULE except_at_lambda ----
\* @ in a function over an infinite domain is the value it replaces.
EXTENDS Integers, TLC
f == [i \in 1..3 |-> 10 * i]
ASSUME PrintT([f EXCEPT ![2] = [i \in Int |-> @ + i]][2][-5])
====
