---- MODULE quantifier_string ----
\* A quantifier over a string.
EXTENDS Integers, FiniteSets, TLC
ASSUME \E x \in "ab" : TRUE
====
