---- MODULE view_symmetry ----
\* With a VIEW and a SYMMETRY, TLC takes the least of a state's
\* permutations, comparing the variables in turn, and fingerprints the
\* view of it (Raft from Vanlightly/raft-tlaplus).
EXTENDS Naturals, TLC
CONSTANTS Procs
VARIABLES pc, steps
Init == pc = [p \in Procs |-> 0] /\ steps = 0
Next == \E p \in Procs : pc[p] < 2 /\ pc' = [pc EXCEPT ![p] = @ + 1] /\ steps' = steps + 1
View == <<pc, steps % 2>>
Sym == Permutations(Procs)
====
CONSTANTS Procs = {p1, p2, p3}
INIT Init
NEXT Next
VIEW View
SYMMETRY Sym
CHECK_DEADLOCK FALSE
