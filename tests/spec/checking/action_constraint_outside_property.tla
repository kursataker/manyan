---- MODULE action_constraint_outside_property ----
\* A step outside the action constraint is checked against the action
\* properties, as every step is.
EXTENDS Naturals
VARIABLES x, y
Init == x = 0 /\ y = 0
Next == \/ x < 6 /\ x' = x + 1 /\ UNCHANGED y
        \/ x' = 9 /\ y' = 7
NoNine == x' /= 9
Spec == Init /\ [][Next]_<<x, y>>
P == [][y' # 7]_<<x, y>>
====
SPECIFICATION Spec
ACTION_CONSTRAINT NoNine
PROPERTY P
