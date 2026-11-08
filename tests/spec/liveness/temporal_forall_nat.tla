---- MODULE temporal_forall_nat ----
\* \A over Nat in a temporal formula: TLC cannot handle it, and says why.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
P == \A y \in Nat : [](x >= 0)
====
INIT Init
NEXT Next
PROPERTY P
