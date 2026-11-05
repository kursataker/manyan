---- MODULE eager_filter ----
\* A filter of an interval is computed at once, so the predicate fails at 0.
EXTENDS Integers, FiniteSets, TLC
ASSUME PrintT(1 \in {x \in 0..2 : 6 \div x > 1})
====
