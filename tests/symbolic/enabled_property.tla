\* options: -length 8 -deadlock
---- MODULE enabled_property ----
\* ENABLED in a property: each process's Step ends disabled, so the fair
\* behavior stutters where none is enabled.
EXTENDS Naturals
VARIABLE
    \* @type: Int -> Int;
    pc
Procs == 1..3
Init == pc = [p \in Procs |-> 0]
Step(p) == pc[p] < 2 /\ (\A q \in Procs : q < p => pc[q] > pc[p]) /\ pc' = [pc EXCEPT ![p] = pc[p] + 1]
Next == \E p \in Procs : Step(p)
Spec == Init /\ [][Next]_pc /\ \A p \in Procs : WF_pc(Step(p))
On == \A p \in Procs : <>[](ENABLED Step(p))
====
SPECIFICATION Spec
PROPERTY On
