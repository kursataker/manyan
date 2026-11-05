---- MODULE choose_tuples ----
\* CHOOSE of a tuple pattern over a set that has a non-tuple.
EXTENDS Integers, FiniteSets, TLC
ASSUME PrintT(CHOOSE <<a, b>> \in {1, 2} : TRUE)
====
