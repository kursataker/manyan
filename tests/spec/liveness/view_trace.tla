---- MODULE view_trace ----
\* -view prints each state of a counterexample as the VIEW's value.
\* options: -view
EXTENDS Naturals
VARIABLE x, y
Init == x = 0 /\ y = 0
A == x < 2 /\ x' = x + 1 /\ UNCHANGED y
B == x = 2 /\ y' = 1 - y /\ UNCHANGED x
Next == A \/ B
Spec == Init /\ [][Next]_<<x, y>> /\ WF_<<x, y>>(Next)
V == [p |-> x, q |-> y]
Live == <>(x = 9)
====
SPECIFICATION Spec
VIEW V
PROPERTY Live
