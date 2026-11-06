---- MODULE is_finite_set_value ----
\* IsFiniteSet of a value that is no set is TLC's error.
EXTENDS Naturals, FiniteSets
ASSUME IsFiniteSet(3)
VARIABLE v
Init == v = 0
Next == UNCHANGED v
====
INIT Init
NEXT Next
