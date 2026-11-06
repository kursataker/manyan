---- MODULE enabled_constant ----
\* A constant definition is computed once, also where ENABLED uses it: a
\* drawn constant is the same value inside ENABLED as outside.
EXTENDS Naturals, TLC
C == <<RandomElement(1..1000000), RandomElement(1..1000000)>>
VARIABLES x, y
Init == x = C /\ y = 0
Next == x = C /\ y < 3 /\ x' = x /\ y' = y + 1
Inv == y < 3 => ENABLED Next
====
INIT Init
NEXT Next
INVARIANT Inv
CHECK_DEADLOCK FALSE
