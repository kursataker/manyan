---- MODULE symmetry_small ----
\* When a symmetry has no more functions than its definition has
\* arguments, TLC warns of each argument, named by its text without
\* spaces.
EXTENDS Naturals, TLC
CONSTANTS P, Q
VARIABLE x
Init == x \in P \cup Q
Next == x' \in P \cup Q
Perms == Permutations(P) \cup Permutations(Q \cup {})
====
CONSTANTS P = {p1}  Q = {q1}
INIT Init
NEXT Next
SYMMETRY Perms
