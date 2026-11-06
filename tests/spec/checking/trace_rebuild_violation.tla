---- MODULE trace_rebuild_violation ----
\* Next fails as TLC rebuilds the behavior to a violated invariant: the
\* behavior ends there, unreported, and the violation is the exit.
EXTENDS Naturals, TLC
VARIABLE x
Init == x \in {0, 1}
Next == x' = x + 1 /\ Assert(TLCGet("distinct") < 4, <<"rebuilt", x>>)
Inv == x < 3
====
INIT Init
NEXT Next
INVARIANT Inv
CHECK_DEADLOCK FALSE
