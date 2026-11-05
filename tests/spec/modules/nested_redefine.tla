---- MODULE nested_redefine ----
\* A nested module defining an operator of the module around it is an error.
EXTENDS Naturals
Op == 1
---- MODULE Inner ----
Op == 2
====
VARIABLE x
Init == x = Op
Next == x' = x
====
INIT Init
NEXT Next
