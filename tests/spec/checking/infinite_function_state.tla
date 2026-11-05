---- MODULE infinite_function_state ----
\* A state cannot hold a function over Nat: TLC fails to count its domain.
EXTENDS Naturals
VARIABLE v
Init == v = [x \in Nat |-> x]
Next == UNCHANGED v
====
INIT Init
NEXT Next
