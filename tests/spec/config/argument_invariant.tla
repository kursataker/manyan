---- MODULE argument_invariant ----
\* The configuration's definitions take no arguments.
VARIABLE x
Init == x = 0
Bounded(n) == x = n
====
INIT Init
NEXT Init
INVARIANT Bounded
