---- MODULE lazy_function_action ----
\* As TLC: a function it keeps lazy, such as a recursive one, applied in an
\* action or the initial predicate is its body there, whose disjunctions
\* are choices counted in the states generated, and which may assign.
EXTENDS Naturals
VARIABLE x
F[n \in 0..2] == IF n = 0 THEN x \in {1, 2} ELSE F[n - 1] \/ x = 7
G[n \in 0..3] == IF n = 0 THEN x' = 0 ELSE \/ x' = n \/ G[n - 1]
Init == F[2]
Next == \/ G[3]
        \/ x < 3 /\ [i \in {1, 2} |-> x' = x + i][1]
====
INIT Init
NEXT Next
