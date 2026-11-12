---- MODULE possible_undefined ----
\* A _POSSIBLE name must be defined.
VARIABLE x
Init == x = 0
====
INIT Init
NEXT Init
_POSSIBLE Nope
