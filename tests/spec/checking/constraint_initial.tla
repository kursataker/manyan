---- MODULE constraint_initial ----
\* The one initial state violates the constraint: 1 state generated,
\* none distinct.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 2 /\ x' = x + 1
Constr == x > 0
====
INIT Init
NEXT Next
CONSTRAINT Constr
