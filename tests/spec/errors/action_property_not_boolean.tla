---- MODULE action_property_not_boolean ----
\* [A]_e whose A is not a boolean.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x
P == [][IF x' < 2 THEN TRUE ELSE 7]_x
====
SPECIFICATION Spec
PROPERTY P
