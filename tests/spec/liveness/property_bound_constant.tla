---- MODULE property_bound_constant ----
\* A property split over \A whose part is the bound value 1.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
P == \A y \in {1} : [](x >= 0) /\ y
====
INIT Init
NEXT Next
PROPERTY P
