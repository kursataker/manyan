---- MODULE set_op_fingerprint ----
\* A set operation of values TLC does not enumerate is kept as its
\* definition, failing only when enumerated; replaying the step, TLC
\* cannot fingerprint it.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == \E m \in (x \cap Nat) : x' = m
====
INIT Init
NEXT Next
