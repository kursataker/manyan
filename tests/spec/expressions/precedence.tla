---- MODULE precedence ----
EXTENDS Integers, TLC
ASSUME PrintT(<<1 + 2 = 3 /\ 2 * 2 = 4, ~ 1 = 2, 1 .. 2 \cup 4 .. 5, - 2 ^ 2, 2 * 3 % 4>>)
ASSUME PrintT(<<1 \in {1} => FALSE, ({1, 2} \ {1}) \cup {3}, TRUE \/ (FALSE /\ FALSE), 2 * 2 + 1 \in {5}>>)
====
