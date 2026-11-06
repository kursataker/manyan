---- MODULE error_successor ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x
P == <>(IF x = 2 THEN x = "s" ELSE x = 3)
====
SPECIFICATION Spec
PROPERTY P
CHECK_DEADLOCK FALSE
