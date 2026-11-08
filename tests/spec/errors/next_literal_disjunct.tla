---- MODULE next_literal_disjunct ----
\* A number among Next's disjuncts: TLC fails as it splits the actions, before the initial states.
VARIABLE x
Init == x = 0
Next == x' = 1 \/ 3
====
INIT Init
NEXT Next
