---- MODULE fairness_implies_false ----
\* A fairness conjunct P => WF_v(A) whose constant P is false asks
\* nothing, so the behavior may stutter.
EXTENDS Naturals
VARIABLE x
Next == x' = (x + 1) % 3
Spec == x = 0 /\ [][Next]_x /\ \A i \in {1, 2} : (i # i) => WF_x(Next)
Two == <>(x = 2)
====
SPECIFICATION Spec
PROPERTY Two
