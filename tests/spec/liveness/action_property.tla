---- MODULE action_property ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 4
Spec == Init /\ [][Next]_x
Increasing == [][x' > x]_x
====
SPECIFICATION Spec
PROPERTY Increasing
