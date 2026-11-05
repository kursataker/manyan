---- MODULE infinite_cardinality ----
\* Counting a set with an infinite part.
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT(Cardinality([{1} -> Nat]))
====
