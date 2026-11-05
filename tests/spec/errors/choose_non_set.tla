---- MODULE choose_non_set ----
\* CHOOSE over a value that is not an enumerable set.
EXTENDS Integers, FiniteSets, TLC
ASSUME PrintT(CHOOSE x \in Nat : x > 3)
====
