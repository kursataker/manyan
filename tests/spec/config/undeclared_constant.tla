---- MODULE undeclared_constant ----
VARIABLE x
Init == x = 1
Next == UNCHANGED x
====
CONSTANT K = 3
INIT Init
NEXT Next
