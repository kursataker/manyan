---- MODULE property_instance_call ----
\* A property that is an instance's operator, with no arguments, is named
\* by the call, as TLC names it.
EXTENDS Naturals
VARIABLE x
Spec == x = 0 /\ [][x' = (x + 1) % 3]_x
C == INSTANCE Counter WITH n <- x
Grows == C!Grows
====
SPECIFICATION Spec
PROPERTY Grows
