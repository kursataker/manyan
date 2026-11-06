---- MODULE continue ----
\* -continue reports a state's first violated invariant, then its step's
\* violated action property, and goes on: the violating states are
\* expanded, and the search ends with no error found, as TLC's does.
\* options: -continue
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 4 /\ x' = x + 1
Spec == Init /\ [][Next]_x
Inv == x # 2
Inv2 == x < 2
P == [][x' # 2]_x
====
SPECIFICATION Spec
INVARIANT Inv Inv2
PROPERTY P
CHECK_DEADLOCK FALSE
