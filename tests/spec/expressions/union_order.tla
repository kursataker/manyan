---- MODULE union_order ----
\* A UNION TLC keeps is taken by quantifiers and set maps one of its sets
\* after another, in order, duplicates too; CHOOSE, filters and functions
\* take it sorted. Once it is counted, or compared with another value, it
\* keeps its elements, and quantifiers take them sorted.
EXTENDS Integers, TLC, FiniteSets
U == UNION {1..2, {4}, {2, 3}}
ASSUME PrintT(<<CHOOSE y \in U : TRUE, CHOOSE y \in U : y > 1>>)
ASSUME \A y \in U : PrintT(y)
ASSUME PrintT({PrintT(y) : y \in U})
ASSUME PrintT(<<[y \in U |-> y], {y \in U : PrintT(y)}, Cardinality(U)>>)
ASSUME \E y \in UNION {1..2, {4, 5}} : PrintT(y) /\ y = 4
ASSUME \A a \in U, b \in UNION {{3}, 1..2} : PrintT(<<a, b>>)
V == UNION {{3}, 1..2}
W == UNION {{6}, 4..5}
ASSUME PrintT(<<V = {1, 2, 3}, {4, 5, 6} = W>>)
ASSUME \A a \in V, b \in W : PrintT(<<a, b>>)
VARIABLE x
Init == x \in UNION {1..2, {4}}
Next == x' \in UNION {{x + 1}, 0..1} /\ x < 5
Inv == x < 5
====
INIT Init
NEXT Next
INVARIANT Inv
