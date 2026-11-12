---- MODULE possible_argument ----
\* A _POSSIBLE predicate takes no argument.
VARIABLE x
Init == x = 0
P(a) == x = a
====
INIT Init
NEXT Init
_POSSIBLE P
