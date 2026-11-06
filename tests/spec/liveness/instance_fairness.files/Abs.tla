---- MODULE Abs ----
EXTENDS Naturals
VARIABLES d, e
Init == d = 0 /\ e = 0
Next == e < 2 /\ e' = e + 1 /\ UNCHANGED d
Spec == Init /\ [][Next]_<<d, e>> /\ WF_<<d, e>>(Next)
====
