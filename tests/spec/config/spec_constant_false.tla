---- MODULE spec_constant_false ----
\* A conjunct of a specification that is a constant FALSE makes it
\* trivial.
VARIABLE x
Init == FALSE
Next == x' = 1
Spec == Init /\ [][Next]_x
====
SPECIFICATION Spec
