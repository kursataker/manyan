---- MODULE constant_not_boolean ----
\* A constant in a temporal property that is no boolean: TLC fails as it builds
\* its tableau, naming the constant.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
P == x ~> 1
====
INIT Init
NEXT Next
PROPERTY P
