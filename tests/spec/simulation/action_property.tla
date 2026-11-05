---- MODULE action_property ----
\* An action property checked on each step.
\* options: -simulate -depth 10 -seed 2
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = x + 1 \/ x' = x + 3
Spec == Init /\ [][Next]_x
Slow == [][x' < x + 2]_x
====
SPECIFICATION Spec
PROPERTY Slow
