---- MODULE quantifier_subset_nat ----
\* A quantifier over SUBSET Nat fails counting Nat.
EXTENDS Integers, FiniteSets, TLC
ASSUME \E x \in SUBSET Nat : TRUE
====
