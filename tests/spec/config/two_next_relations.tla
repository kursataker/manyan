---- MODULE two_next_relations ----
\* A specification with two conjuncts [][A]_v is one TLC cannot check,
\* though both are the same action.
EXTENDS Naturals, TLC
VARIABLE x
ASSUME PrintT("assumed")
Init == x = 0
Next == x < 3 /\ x' = x + 1
Box == [][Next]_x
Spec == Init /\ Box /\ [][Next]_x
====
SPECIFICATION Spec
