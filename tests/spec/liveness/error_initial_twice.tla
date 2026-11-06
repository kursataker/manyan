---- MODULE error_initial_twice ----
EXTENDS Naturals
VARIABLE x
Init == x \in {7, 8, 9}
Inv == x # 8
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x
P == <>(IF x >= 7 THEN x = "s" ELSE x = 3)
====
SPECIFICATION Spec
PROPERTY P
INVARIANT Inv
CHECK_DEADLOCK FALSE
