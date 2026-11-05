---- MODULE counters ----
\* Independent counters: a wide state space of small integer functions.
EXTENDS Naturals
CONSTANTS Procs, Max
VARIABLE c
Init == c = [p \in Procs |-> 0]
Inc(p) == c[p] < Max /\ c' = [c EXCEPT ![p] = @ + 1]
Reset(p) == c[p] = Max /\ c' = [c EXCEPT ![p] = 0]
Next == \E p \in Procs : Inc(p) \/ Reset(p)
Bounded == \A p \in Procs : c[p] <= Max
====
CONSTANTS Procs = {p1, p2, p3, p4, p5, p6} Max = 6
INIT Init
NEXT Next
INVARIANT Bounded
