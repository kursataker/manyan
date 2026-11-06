---- MODULE unbounded_choose ----
\* An unbounded CHOOSE evaluated: TLC 2.19's message.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (CHOOSE n : n \notin Nat)
====
INIT Init
NEXT Next
