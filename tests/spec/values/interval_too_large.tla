---- MODULE interval_too_large ----
\* An interval of 2^31 elements or more is TLC's error only where its size
\* is needed: membership and a quantifier over it go one by one (tlx's
\* UnionTailComparison, where tpp ran out of memory).
EXTENDS Naturals, TLC
Big == 0..2147483647
ASSUME PrintT(<<7 \in Big, \E x \in Big : x = 3>>)
ASSUME PrintT(Big = {})
====
