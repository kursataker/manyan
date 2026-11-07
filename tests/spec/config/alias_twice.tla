---- MODULE alias_twice ----
\* ALIAS names one definition, once.
VARIABLE x
Init == x = 0
====
INIT Init
NEXT Init
ALIAS Init
ALIAS Init
