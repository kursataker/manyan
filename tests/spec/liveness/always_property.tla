---- MODULE always_property ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 4
Spec == Init /\ [][Next]_x
Bounded == [](x < 3)
====
SPECIFICATION Spec
PROPERTY Bounded
