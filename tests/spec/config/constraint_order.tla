---- MODULE constraint_order ----
\* TLC reads the properties before the constraints, whatever their order
\* in the configuration.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = 1 - x
C == FALSE
P == FALSE
====
INIT Init
NEXT Next
CONSTRAINT C
PROPERTY P
