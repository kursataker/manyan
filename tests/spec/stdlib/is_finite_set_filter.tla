---- MODULE is_finite_set_filter ----
\* IsFiniteSet of a filter over a set that is not finite is TLC's error,
\* even when the filter is empty.
EXTENDS Naturals, FiniteSets
ASSUME ~IsFiniteSet({x \in Nat : FALSE})
VARIABLE v
Init == v = 0
Next == UNCHANGED v
====
INIT Init
NEXT Next
