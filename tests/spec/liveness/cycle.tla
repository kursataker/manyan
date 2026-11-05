---- MODULE cycle ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == \/ x < 3 /\ x' = x + 1
        \/ x = 1 /\ x' = 0
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Reach == <>(x = 3)
Often == []<>(x = 0)
====
SPECIFICATION Spec
PROPERTY Reach
CHECK_DEADLOCK FALSE
