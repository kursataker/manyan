---- MODULE lazy_outside_domain ----
\* Applied outside its domain, a lazy function that cannot be computed in
\* full is named by its definition.
EXTENDS Integers, TLC
ASSUME PrintT([x0 \in 1..2 |-> [x1 \in 1..2 |-> 3][3]][3])
====
