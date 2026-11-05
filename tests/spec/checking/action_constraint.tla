---- MODULE action_constraint ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 6 /\ (x' = x + 1 \/ x' = x + 2)
OnlyOdd == x' - x = 1 \/ x' % 2 = 0
====
INIT Init
NEXT Next
ACTION_CONSTRAINT OnlyOdd
CHECK_DEADLOCK FALSE
