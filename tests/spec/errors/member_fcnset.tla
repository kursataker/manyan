---- MODULE member_fcnset ----
\* Only a function may be in a set of functions.
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT(1 \in [{1} -> {1}])
====
