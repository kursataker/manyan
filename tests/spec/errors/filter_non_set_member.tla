---- MODULE filter_non_set_member ----
\* Membership in a filter of a non-set.
EXTENDS Integers, FiniteSets, TLC
ASSUME PrintT(1 \in {x \in 3 : TRUE})
====
