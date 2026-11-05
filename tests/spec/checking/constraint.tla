---- MODULE constraint ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = x + 1 \/ x' = x + 2
Bound == x < 10
====
INIT Init
NEXT Next
CONSTRAINT Bound
