---- MODULE partial_fairness ----
EXTENDS Naturals
VARIABLE c
Init == c = [p \in {1, 2} |-> 0]
Inc(p) == c[p] < 2 /\ (p = 2 => c[1] = 2) /\ c' = [c EXCEPT ![p] = @ + 1]
Spec == Init /\ [][\E p \in {1, 2} : Inc(p)]_c /\ WF_c(Inc(1))
Done == <>(c[2] >= 1)
====
SPECIFICATION Spec
PROPERTY Done
CHECK_DEADLOCK FALSE
