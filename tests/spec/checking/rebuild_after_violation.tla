---- MODULE rebuild_after_violation ----
\* After reporting a violation TLC rebuilds the behavior again, for a
\* POSTCONDITION, printing what Init and Next print once more.
EXTENDS Naturals, TLC
VARIABLE x
Init == PrintT("init") /\ x = 0
Next == PrintT(<<"next", x>>) /\ x' = x + 1
Inv == x < 2
====
INIT Init
NEXT Next
INVARIANT Inv
