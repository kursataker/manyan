---- MODULE temporal_forall ----
\* TLC cannot check temporal universal quantification.
EXTENDS Naturals
VARIABLE v
Init == v = 0
Next == v' = 1 - v
Spec == Init /\ [][Next]_v
P == \AA x : <>(v = 0)
====
SPECIFICATION Spec
PROPERTY P
