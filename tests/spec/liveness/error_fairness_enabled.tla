---- MODULE error_fairness_enabled ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
A == x < 3 /\ x' = x + 1
Next == A
Spec == Init /\ [][Next]_x /\ WF_x(A /\ (x = 1 => 1 \div 0 = 0))
P == <>(x = 3)
====
SPECIFICATION Spec
PROPERTY P
CHECK_DEADLOCK FALSE
