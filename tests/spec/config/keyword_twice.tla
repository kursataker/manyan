---- MODULE keyword_twice ----
\* INIT, NEXT, SPECIFICATION, SYMMETRY, VIEW and CHECK_DEADLOCK name one
\* thing each, once.
VARIABLE x
Init == x = 0
====
INIT Init
NEXT Init
CHECK_DEADLOCK FALSE
CHECK_DEADLOCK TRUE
