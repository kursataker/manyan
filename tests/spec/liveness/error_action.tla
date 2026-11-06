---- MODULE error_action ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x
P == []<><<x = 1 /\ x' = "s">>_x
====
SPECIFICATION Spec
PROPERTY P
CHECK_DEADLOCK FALSE
