---- MODULE interval_too_large ----
\* An interval of 2^31 elements or more is TLC's error only where its size
\* is needed: membership goes without it, a quantifier over it does not.
EXTENDS Naturals, TLC
Big == 0..2147483647
ASSUME PrintT(7 \in Big)
ASSUME PrintT(\E x \in Big : x = 3)
====
