---- MODULE symmetry ----
EXTENDS TLC
CONSTANT P
VARIABLE owner
Init == owner = "none"
Next == \/ owner = "none" /\ \E p \in P : owner' = p
        \/ owner # "none" /\ owner' = "none"
Sym == Permutations(P)
====
CONSTANT P = {p1, p2, p3}
INIT Init
NEXT Next
SYMMETRY Sym
