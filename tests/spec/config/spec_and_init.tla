---- MODULE spec_and_init ----
VARIABLE x
Init == x = 1
Next == UNCHANGED x
Spec == Init /\ [][Next]_x
====
SPECIFICATION Spec
INIT Init
NEXT Next
