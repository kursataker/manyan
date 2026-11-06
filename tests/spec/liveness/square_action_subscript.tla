---- MODULE square_action_subscript ----
\* [A]_e evaluates e only when A is FALSE, and <<A>>_e only when A is
\* TRUE, as TLC does: here e would fail.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x
P == [][x' > x]_(1 \div 0)
Q == [][~<<x' < x>>_(1 \div 0)]_x
====
SPECIFICATION Spec
PROPERTY P Q
CHECK_DEADLOCK FALSE
