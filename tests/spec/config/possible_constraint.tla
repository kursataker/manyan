---- MODULE possible_constraint ----
\* A state _POSSIBLE predicate is a constraint after the others, evaluated
\* only where they hold; an action one, only on a step from a state
\* in the model.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = x + 1
Small == x < 2
Two == x = 2
Three == x = 3
Jump == x' = 3
====
INIT Init
NEXT Next
_POSSIBLE Two Jump Three
CONSTRAINT Small
CHECK_DEADLOCK FALSE
