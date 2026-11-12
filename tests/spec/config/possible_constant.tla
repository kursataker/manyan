---- MODULE possible_constant ----
\* A _POSSIBLE name cannot be a constant.
CONSTANT C
VARIABLE x
Init == x = 0
====
CONSTANT C = 1
INIT Init
NEXT Init
_POSSIBLE C
