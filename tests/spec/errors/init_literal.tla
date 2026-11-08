---- MODULE init_literal ----
\* A number where TLC expects the initial predicate.
VARIABLE x
Init == 3 /\ x = 1
Next == x' = x
====
INIT Init
NEXT Next
