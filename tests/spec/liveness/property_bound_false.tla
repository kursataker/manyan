---- MODULE property_bound_false ----
\* A property split over \A whose part is the bound value FALSE.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
P == \A y \in {FALSE} : [](x >= 0) /\ y
====
INIT Init
NEXT Next
PROPERTY P
