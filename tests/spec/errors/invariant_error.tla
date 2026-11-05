---- MODULE invariant_error ----
\* An invariant that cannot be evaluated in a new state.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = x + 1 /\ x < 3
Inv == 10 \div (2 - x) > 0 - 100
====
INIT Init
NEXT Next
INVARIANT Inv
