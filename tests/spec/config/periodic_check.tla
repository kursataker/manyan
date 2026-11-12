---- MODULE periodic_check ----
\* Model checking evaluates _PERIODIC only at a progress report, which
\* comes once a minute.
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Periodic == PrintT(TLCGet("stats").generated) /\ FALSE
====
INIT Init
NEXT Next
_PERIODIC Periodic
CHECK_DEADLOCK FALSE
