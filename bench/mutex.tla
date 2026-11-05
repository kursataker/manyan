---- MODULE mutex ----
\* Peterson-style mutual exclusion for N processes (filter lock): sets,
\* quantifiers and function updates.
EXTENDS Naturals
CONSTANT N
Procs == 1..N
VARIABLES level, victim, pc
Init == level = [p \in Procs |-> 0] /\ victim = [l \in 1..N-1 |-> 1] /\ pc = [p \in Procs |-> "idle"]
Try(p) == pc[p] = "idle" /\ pc' = [pc EXCEPT ![p] = "wait"] /\ level' = [level EXCEPT ![p] = 1]
          /\ victim' = [victim EXCEPT ![1] = p]
Climb(p) == /\ pc[p] = "wait"
            /\ \/ victim[level[p]] # p
               \/ \A q \in Procs \ {p} : level[q] < level[p]
            /\ IF level[p] = N - 1
               THEN pc' = [pc EXCEPT ![p] = "cs"] /\ UNCHANGED <<level, victim>>
               ELSE /\ level' = [level EXCEPT ![p] = @ + 1]
                    /\ victim' = [victim EXCEPT ![level[p] + 1] = p]
                    /\ UNCHANGED pc
Exit(p) == pc[p] = "cs" /\ pc' = [pc EXCEPT ![p] = "idle"] /\ level' = [level EXCEPT ![p] = 0] /\ UNCHANGED victim
Next == \E p \in Procs : Try(p) \/ Climb(p) \/ Exit(p)
Mutex == \A p, q \in Procs : p # q => ~(pc[p] = "cs" /\ pc[q] = "cs")
====
CONSTANT N = 4
INIT Init
NEXT Next
INVARIANT Mutex
