---- MODULE properties ----
\* PROPERTIES lists temporal properties; a spec's fairness comes with it.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Visits == []<>(x = 2)
Bounded == [](x < 3)
====
SPECIFICATION Spec
PROPERTIES Visits Bounded
