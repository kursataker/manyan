---- MODULE if_temporal ----
\* A temporal IF is (p /\ A) \/ (~p /\ B): p is decided in the first state.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 2 /\ x' = x + 1
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Holds == IF x = 0 THEN <>(x = 2) ELSE <>(x = 5)
Fails == IF x # 0 THEN <>(x = 2) ELSE <>(x = 5)
====
SPECIFICATION Spec
PROPERTY Holds Fails
CHECK_DEADLOCK FALSE
