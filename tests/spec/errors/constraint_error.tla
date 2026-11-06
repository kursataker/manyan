---- MODULE constraint_error ----
\* A constraint that fails to evaluate on a successor: TLC reports an
\* unexpected exception, without the behavior.
EXTENDS Naturals
VARIABLE x
R == [n \in 0..2 |-> n]
Init == x = 0
Next == x' = x + 1
C == R[x] = x
====
INIT Init
NEXT Next
CONSTRAINT C
