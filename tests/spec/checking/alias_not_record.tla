---- MODULE alias_not_record ----
\* An alias that is not a record leaves the state as it is.
EXTENDS Naturals
VARIABLE x, y
Init == x = 0 /\ y = 0
Next == x' = x + 1 /\ y' = y
Inv == x < 1
Alias == x
====
INIT Init
NEXT Next
INVARIANT Inv
ALIAS Alias
