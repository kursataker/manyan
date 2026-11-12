---- MODULE action_constraint ----
\* The step to x = 2 is outside the action constraint: x = 2 is checked,
\* x = 3 never reached.
EXTENDS Naturals
VARIABLE
    \* @type: Int;
    x
Init == x = 0
Next == x' = x + 1
NotTwo == x' # 2
Inv == x < 3
====
INIT Init
NEXT Next
ACTION_CONSTRAINT NotTwo
INVARIANT Inv
