---- MODULE instance_spec_nested ----
\* A specification through two instances, its Init, Next and fairness
\* split as one.
VARIABLE y, z
INSTANCE Mid
Inv == y < 2
====
SPECIFICATION Refinement
INVARIANT Inv
