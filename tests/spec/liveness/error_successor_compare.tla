---- MODULE error_successor_compare ----
\* TLC checks the actions of a property on a state's steps before its
\* state predicates on the successors: here <<A>>_x fails first.
VARIABLES x, y
Init == x = "ok" /\ y = 0
A == y = 0 /\ x' = <<1>> /\ y' = 1
B == y = 0 /\ x' = "no" /\ y' = 1
Next == A \/ B
Spec == Init /\ [][Next]_<<x, y>> /\ WF_x(A)
P == <>(x # "ok")
====
SPECIFICATION Spec
PROPERTY P
CHECK_DEADLOCK FALSE
