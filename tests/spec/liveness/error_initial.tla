---- MODULE error_initial ----
EXTENDS Naturals
VARIABLE x
Init == x \in {0, 7}
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x
P == <>(IF x = 7 THEN x = "s" ELSE x = 3)
====
SPECIFICATION Spec
PROPERTY P
CHECK_DEADLOCK FALSE
