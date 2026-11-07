---- MODULE alias_one_field ----
\* An alias of one field prints as a single variable does.
EXTENDS Naturals
VARIABLE x, y
Init == x = 0 /\ y = 0
Next == x' = x + 1 /\ y' = y
Inv == x < 1
Alias == [z |-> x]
====
INIT Init
NEXT Next
INVARIANT Inv
ALIAS Alias
