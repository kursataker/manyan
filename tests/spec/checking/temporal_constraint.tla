---- MODULE temporal_constraint ----
\* A temporal formula evaluated as a state predicate is TLC's error, which
\* names the operator.
EXTENDS Naturals
VARIABLE x
Init == x = 1
Next == x' = x + 1 /\ x < 3
Bounded == (x > 0) ~> (x > 1)
====
INIT Init
NEXT Next
CONSTRAINT Bounded
