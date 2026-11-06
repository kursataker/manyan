---- MODULE constraint_outside_invariant ----
\* A state outside the constraint is not kept, but TLC checks its
\* invariants each time it is found: one found late violates Inv.
EXTENDS Naturals
VARIABLES x, y
Init == x = 0 /\ y = 0
Next == \/ x < 6 /\ x' = x + 1 /\ UNCHANGED y
        \/ y < 6 /\ y' = y + 1 /\ UNCHANGED x
        \/ x' = 9 /\ y' = y % 4
Constraint == x /= 9
Inv == x = 9 => y < 3
====
INIT Init
NEXT Next
CONSTRAINT Constraint
INVARIANT Inv
