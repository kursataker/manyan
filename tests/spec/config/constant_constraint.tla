---- MODULE constant_constraint ----
\* A constant constraint is evaluated as the configuration is read, as a
\* constant invariant is.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = 1 - x
C == FALSE
====
INIT Init
NEXT Next
CONSTRAINT C
