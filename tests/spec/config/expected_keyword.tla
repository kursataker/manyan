---- MODULE expected_keyword ----
\* TLC reads the configuration before the module, and ALIAS is not one of
\* its keywords: the error is the configuration's, at its line.
VARIABLE x
Init == x = Undefined
====
INIT Init
NEXT Init
ALIAS Init
