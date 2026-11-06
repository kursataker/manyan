---- MODULE implied_init_error ----
\* An error evaluating a property that is a state predicate, in an
\* initial state.
EXTENDS Naturals
VARIABLE x
Init == x \in {0, 1}
Next == x' = 1 - x
P == x = {}
====
INIT Init
NEXT Next
PROPERTY P
