---- MODULE shadowing ----
\* A bound name may not hide a definition.
EXTENDS Naturals
VARIABLE x
n == 3
Init == x \in {n \in 1..2 : TRUE}
Next == x' = x
====
INIT Init
NEXT Next
