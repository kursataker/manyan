---- MODULE action_constraint_not_boolean ----
\* An action constraint that is not a boolean is TLC's error.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
C == IF x' < 2 THEN TRUE ELSE 7
====
INIT Init
NEXT Next
ACTION_CONSTRAINT C
