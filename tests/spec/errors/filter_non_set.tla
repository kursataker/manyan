---- MODULE filter_non_set ----
\* Enumerating a filter of a non-set.
EXTENDS Integers, FiniteSets, TLC
ASSUME PrintT({x \in 3 : TRUE} = {})
====
