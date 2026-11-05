---- MODULE infinitely_often ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == \/ x' = (x + 1) % 3
        \/ x = 2 /\ x' = 2
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Often == []<>(x = 0)
Settles == <>[](x = 2)
====
SPECIFICATION Spec
PROPERTIES Often
