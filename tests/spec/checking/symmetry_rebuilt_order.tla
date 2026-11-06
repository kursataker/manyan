---- MODULE symmetry_rebuilt_order ----
\* Under symmetry a trace shows the states as rebuilt, normalized as TLC
\* keeps them: a function of two arguments in its keys' order. Found by
\* tlaplus/Examples' MCCheckpointCoordinationFailure.
EXTENDS Naturals, TLC
CONSTANT Node
VARIABLE p, c
Init == p = [s, d \in Node |-> TRUE] /\ c = 0
Next == c < 2 /\ c' = c + 1 /\ UNCHANGED p
TypeOK == c < 2
Sym == Permutations(Node)
====
CONSTANT Node = {n1, n2, n3}
INIT Init
NEXT Next
INVARIANT TypeOK
SYMMETRY Sym
