---- MODULE possible ----
\* Each _POSSIBLE predicate held somewhere: no error.
EXTENDS Naturals
VARIABLE x
Init == x \in {0, 1}
Next == x < 3 /\ x' = x + 1
Two == x = 2
Step == x' = x + 1 /\ x = 1
====
INIT Init
NEXT Next
_POSSIBLE Two Step
CHECK_DEADLOCK FALSE
