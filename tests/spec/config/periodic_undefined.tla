---- MODULE periodic_undefined ----
\* A _PERIODIC name must be defined.
VARIABLE x
Init == x = 0
====
INIT Init
NEXT Init
_PERIODIC Nope
