---- MODULE missing_constant ----
CONSTANT K
VARIABLE x
Init == x = K
Next == UNCHANGED x
====
INIT Init
NEXT Next
