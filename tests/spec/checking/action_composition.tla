---- MODULE action_composition ----
\* TLC does not evaluate action composition.
EXTENDS Naturals
VARIABLE x
Init == x = 1
Next == (x' = x + 1) \cdot (x' = x + 1)
====
INIT Init
NEXT Next
