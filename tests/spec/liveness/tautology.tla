---- MODULE tautology ----
\* A property whose negation its constants make FALSE is a tautology, which
\* TLC does not check; 1 = 1 is a constant, TRUE.
EXTENDS Naturals
VARIABLE x
Spec == x = 0 /\ [][x' = 1 - x]_x
Prop == <>(x = 7) \/ (1 = 1)
====
SPECIFICATION Spec
PROPERTY Prop
