---- MODULE no_next ----
\* A specification without steps: with no invariant or property to check,
\* TLC computes the initial states, then stops.
VARIABLE x
Init == x \in {1, 2}
Spec == Init
====
SPECIFICATION Spec
