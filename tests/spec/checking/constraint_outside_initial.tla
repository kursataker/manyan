---- MODULE constraint_outside_initial ----
\* An initial state outside the constraint is checked against the
\* invariants too.
EXTENDS Naturals
VARIABLES x
Init == x \in {0, 9}
Next == x < 3 /\ x' = x + 1
Constraint == x /= 9
Inv == x # 9
====
INIT Init
NEXT Next
CONSTRAINT Constraint
INVARIANT Inv
