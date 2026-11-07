---- MODULE N ----
EXTENDS Naturals
VARIABLE x, z
vars == <<x, z>>
Init == x = 0 /\ z = 0
A == x' = (x + 1) % 3 /\ UNCHANGED z
B == z' = 1 - z /\ UNCHANGED x
Next == A \/ B
Spec == Init /\ [][Next]_vars /\ WF_vars(Next)
====
