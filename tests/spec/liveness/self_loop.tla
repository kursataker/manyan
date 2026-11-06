---- MODULE self_loop ----
\* A cycle of one state that an action returns to is printed as
\* stuttering, as TLC prints it.
EXTENDS Naturals
VARIABLE x
Init == x = 1
Next == x' = x
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Reaches2 == <>(x = 2)
====
SPECIFICATION Spec
PROPERTY Reaches2
