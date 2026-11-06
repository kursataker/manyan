---- MODULE error_initial_tableau ----
EXTENDS Naturals
VARIABLE x
Init == x \in {0, 7}
Inv == x # 7
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x
P == [](x = 0 => <>(IF x = 7 THEN x = "s" ELSE TRUE))
====
SPECIFICATION Spec
PROPERTY P
INVARIANT Inv
CHECK_DEADLOCK FALSE
