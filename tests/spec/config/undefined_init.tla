---- MODULE undefined_init ----
VARIABLE x
Init == x = 1
Next == UNCHANGED x
====
INIT Start
NEXT Next
