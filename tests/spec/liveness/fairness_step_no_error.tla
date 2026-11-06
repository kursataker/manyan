---- MODULE fairness_step_no_error ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
A == x < 3 /\ x' = x + 1
Bad == x = 2 /\ x' = "s"
Next == A
Spec == Init /\ [][Next]_x /\ WF_x(A \/ Bad)
P == <>(x = 3)
====
SPECIFICATION Spec
PROPERTY P
CHECK_DEADLOCK FALSE
