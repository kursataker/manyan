---- MODULE possible_never ----
\* A _POSSIBLE predicate never witnessed fails after the search, and TLC
\* prints the counts, by name, of those it evaluated.
EXTENDS Naturals
VARIABLE x
Init == x \in {0, 1}
Next == x < 3 /\ x' = x + 1
Zeta == x = 2
Big == x = 7
Step == x' = x + 1 /\ x = 1
Alpha == x = 0
====
INIT Init
NEXT Next
_POSSIBLE Zeta Big Step Alpha
CHECK_DEADLOCK FALSE
