---- MODULE possible_temporal ----
\* A _POSSIBLE predicate is no temporal formula.
VARIABLE x
Init == x = 0
P == <>(x = 1)
====
INIT Init
NEXT Init
_POSSIBLE P
