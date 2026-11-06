---- MODULE undefined_invariant ----
VARIABLE x
Init == x = 1
Next == UNCHANGED x
====
INIT Init
NEXT Next
INVARIANT Missing
