---- MODULE lazy_argument_list ----
\* A lazy function of two bounds applied to a non-tuple.
EXTENDS Integers, TLC
ASSUME PrintT([x \in 1..2, y \in 1..3 |-> x][7])
====
