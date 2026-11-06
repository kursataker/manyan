---- MODULE error_successor_compare ----
VARIABLE x
Init == x = "ok"
A == x = "ok" /\ x' = <<1>>
B == x = "ok" /\ x' = "no"
Next == A \/ B
Spec == Init /\ [][Next]_x /\ WF_x(A)
P == <>(x # "ok")
====
SPECIFICATION Spec
PROPERTY P
CHECK_DEADLOCK FALSE
