---- MODULE C ----
EXTENDS Naturals
VARIABLE t
Init == t = 0
Next == t' = t + 1
Spec == Init /\ [][Next]_t
====
