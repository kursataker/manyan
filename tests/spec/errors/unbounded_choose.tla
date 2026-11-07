---- MODULE unbounded_choose ----
\* An unbounded CHOOSE evaluated: TLC's message.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (CHOOSE n : n \notin Nat)
====
INIT Init
NEXT Next
