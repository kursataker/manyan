---- MODULE inline_action ----
\* A specification's step that is no named operator is TLC's Action.
EXTENDS Naturals
VARIABLE x
Spec == x = 0 /\ [][x < 2 /\ x' = x + 1]_x
Small == x < 2
====
SPECIFICATION Spec
INVARIANT Small
