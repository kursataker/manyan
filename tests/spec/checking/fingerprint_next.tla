---- MODULE fingerprint_next ----
\* A successor's nested function fails: each constructor being computed is
\* in the fingerprint stack trace, innermost first.
EXTENDS Integers
VARIABLE v
Init == v = 0
Next == v < 1 /\ v' = [y \in 1..2 |-> [x \in 0..2 |-> CHOOSE z \in {x} : z > 1]]
====
INIT Init
NEXT Next
