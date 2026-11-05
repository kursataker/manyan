---- MODULE eventually_always ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Settles == <>[](x = 3)
Unsettled == []<>(x = 0)
====
SPECIFICATION Spec
PROPERTY Settles Unsettled
CHECK_DEADLOCK FALSE
