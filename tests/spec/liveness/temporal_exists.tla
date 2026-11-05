---- MODULE temporal_exists ----
\* TLC cannot check temporal existential quantification.
EXTENDS Naturals
VARIABLE v
Init == v = 0
Next == v' = 1 - v
Spec == Init /\ [][Next]_v
P == \EE x : []<>(v = 1)
====
SPECIFICATION Spec
PROPERTY P
