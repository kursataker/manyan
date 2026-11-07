---- MODULE postcondition ----
\* After the search TLC checks the POSTCONDITIONs in order, with no state,
\* and reports the first that is false instead of success.
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Counted == TLCGet("distinct") = 4 /\ PrintT("counted")
Less == TLCGet("distinct") = 3 /\ PrintT("less")
====
INIT Init
NEXT Next
POSTCONDITION Counted
POSTCONDITIONS Less
CHECK_DEADLOCK FALSE
