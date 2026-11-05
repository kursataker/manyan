---- MODULE choose_subset_nat ----
\* CHOOSE over SUBSET Nat fails counting Nat.
EXTENDS Integers, FiniteSets, TLC
ASSUME PrintT(CHOOSE x \in SUBSET Nat : TRUE)
====
