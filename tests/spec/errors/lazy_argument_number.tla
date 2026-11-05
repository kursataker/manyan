---- MODULE lazy_argument_number ----
\* A lazy function of two bounds names the argument outside its set.
EXTENDS Integers, TLC
ASSUME PrintT([x \in 1..2, y \in 1..3 |-> x + y][1, 4])
====
