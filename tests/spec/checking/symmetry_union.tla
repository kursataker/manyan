---- MODULE symmetry_union ----
\* A SYMMETRY that is no group, as Permutations(A) \cup Permutations(B):
\* TLC takes the group it generates.
EXTENDS TLC
CONSTANTS A, B
VARIABLES x, y
Init == x \in A /\ y \in B
Next == \/ \E a \in A : x' = a /\ UNCHANGED y
        \/ \E b \in B : y' = b /\ UNCHANGED x
        \/ x' = x /\ y' = x
Sym == Permutations(A) \cup Permutations(B)
====
CONSTANTS A = {a1, a2, a3}
          B = {b1, b2}
INIT Init
NEXT Next
SYMMETRY Sym
