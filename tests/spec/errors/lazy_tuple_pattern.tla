---- MODULE lazy_tuple_pattern ----
\* A lazy function over a tuple pattern applied outside its set.
EXTENDS Integers, TLC
ASSUME PrintT([<<a, b>> \in {<<1, 2>>} |-> a][<<1, 3>>])
====
