---- MODULE temporal_exists_non_set ----
\* \E over a non-set in a temporal formula: TLC cannot handle it, saying only
\* where.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
P == \E y \in 1 : [](x >= 0)
====
INIT Init
NEXT Next
PROPERTY P
