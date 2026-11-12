---- MODULE periodic_argument ----
\* A _PERIODIC definition takes no argument.
VARIABLE x
Init == x = 0
P(a) == a
====
INIT Init
NEXT Init
_PERIODIC P
