---- MODULE union_infinite ----
\* UNION of a set holding an infinite set is kept as UNION S.
EXTENDS Integers, FiniteSets, TLC
ASSUME PrintT(1 \in {} \ UNION {{-1}, Nat})
ASSUME PrintT(Cardinality(UNION {[0..1 -> Nat]}))
====
