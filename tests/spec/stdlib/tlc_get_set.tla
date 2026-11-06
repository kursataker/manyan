---- MODULE tlc_get_set ----
\* TLCSet and TLCGet: a register, and the level of the state.
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0 /\ TLCSet(1, 10)
Next == x < 3 /\ x' = x + 1 /\ TLCGet("level") = x + 1 /\ TLCGet(1) = 10
====
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
