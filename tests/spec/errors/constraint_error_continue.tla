---- MODULE constraint_error_continue ----
\* options: -continue
\* With -continue TLC reports a constraint failing on a successor again as it
\* replays the step.
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
