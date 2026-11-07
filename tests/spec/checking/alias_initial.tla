---- MODULE alias_initial ----
\* An initial state that violates an invariant prints as its alias too.
EXTENDS Naturals
VARIABLE x, y
Init == x = 0 /\ y = 0
Next == x' = x + 1 /\ y' = y
Inv == x > 0
Alias == [z |-> x, n |-> x']
====
INIT Init
NEXT Next
INVARIANT Inv
ALIAS Alias
