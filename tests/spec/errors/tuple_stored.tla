---- MODULE tuple_stored ----
\* A tuple taken from a stored state is applied outside its domain: TLC
\* no longer knows the expression that made it.
EXTENDS Naturals
VARIABLE t, n
Init == t = <<1, 2>> /\ n = 1
Next == n' = n + 1 /\ t' = t /\ t[n + 1] > 0
====
INIT Init
NEXT Next
