---- MODULE model_value_set ----
\* A constant that is a set of model values, under symmetry.
EXTENDS Naturals, TLC
CONSTANT Procs
VARIABLE owner
Init == owner = "none"
Next == \/ owner = "none" /\ \E p \in Procs : owner' = p
        \/ owner \in Procs /\ owner' = "none"
Sym == Permutations(Procs)
====
CONSTANT Procs = {p1, p2, p3}
INIT Init
NEXT Next
SYMMETRY Sym
