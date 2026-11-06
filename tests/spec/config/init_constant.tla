---- MODULE init_constant ----
\* TLC computes a constant definition before it reads the configuration:
\* one named as the initial predicate is no predicate.
VARIABLE x
Init == TRUE
Next == x' = 1
====
INIT Init
NEXT Next
