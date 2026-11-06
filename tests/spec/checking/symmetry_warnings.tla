---- MODULE symmetry_warnings ----
\* TLC warns of a set of a symmetry that permutes nothing, naming it by
\* the constant the configuration replaces with it; and of symmetry with a
\* temporal property. The first warning alone says how to turn them off.
EXTENDS Naturals, TLC
CONSTANTS Procs, Lone, Alone
VARIABLE x
Init == x \in Procs
Next == x' \in Procs
Spec == Init /\ [][Next]_x /\ WF_x(Next)
One == {Lone}
Perms == Permutations(Procs) \cup Permutations(One)
Live == <>(x \in Procs)
Bounded == x \in Procs
====
CONSTANTS Procs = {p1, p2}  Lone = l1  Alone <- One
SPECIFICATION Spec
SYMMETRY Perms
PROPERTY Live
CONSTRAINT Bounded
