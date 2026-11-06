---- MODULE action_property_error ----
\* An action property that fails to evaluate on a step.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = x + 1
Spec == Init /\ [][Next]_x
P == [][1 \div (3 - x') >= 0]_x
====
SPECIFICATION Spec
PROPERTY P
