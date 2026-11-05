---- MODULE wf_holds ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Done == <>(x = 3)
====
SPECIFICATION Spec
PROPERTY Done
CHECK_DEADLOCK FALSE
