---- MODULE unbounded_quantifier ----
\* Evaluating an unbounded quantifier is an error naming it.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (\A n : n = 1)
====
INIT Init
NEXT Next
