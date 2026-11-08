---- MODULE always_constant_not_boolean ----
\* []1 is checked as an invariant: TLC names its body, not the property.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
P == []1
====
INIT Init
NEXT Next
PROPERTY P
