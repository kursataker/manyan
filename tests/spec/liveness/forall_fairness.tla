---- MODULE forall_fairness ----
EXTENDS Naturals
CONSTANT Procs
VARIABLE c
Init == c = [p \in Procs |-> 0]
Inc(p) == c[p] < 2 /\ c' = [c EXCEPT ![p] = @ + 1]
Spec == Init /\ [][\E p \in Procs : Inc(p)]_c /\ \A p \in Procs : WF_c(Inc(p))
Done == \A p \in Procs : <>(c[p] = 2)
====
CONSTANT Procs = {a, b}
SPECIFICATION Spec
PROPERTY Done
CHECK_DEADLOCK FALSE
