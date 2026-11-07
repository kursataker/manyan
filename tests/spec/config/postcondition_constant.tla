---- MODULE postcondition_constant ----
\* TLC resolves the postconditions' names once the search is done: one
\* that is a constant fails then, before any is checked.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Post == FALSE
====
INIT Init
NEXT Next
POSTCONDITION Post
CHECK_DEADLOCK FALSE
