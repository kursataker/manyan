---- MODULE property_part_name ----
\* A part of a property that calls a definition is named by it, as TLC
\* names it: the initial predicate Init2, not its location.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 2 /\ x' = x + 1
Spec == Init /\ [][Next]_x
Init2 == x = 1
P == Init2 /\ [][x' > x]_x
====
SPECIFICATION Spec
PROPERTY P
CHECK_DEADLOCK FALSE
