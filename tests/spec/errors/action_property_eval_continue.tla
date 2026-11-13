---- MODULE action_property_eval_continue ----
\* options: -continue
\* With -continue an action property that fails to evaluate is reported
\* again as an error of the step, and all of it again as TLC replays it.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 2 /\ x' = x + 1
Prop == [][x' < 2 \/ [a |-> 1].b = 1]_x
====
INIT Init
NEXT Next
PROPERTY Prop
