---- MODULE trace_rebuild_fails ----
\* An invariant fails to evaluate, and so does Next as TLC rebuilds the
\* behavior to it: the behavior ends there, and that error is the exit.
EXTENDS Naturals, TLC
VARIABLE x
Init == x \in {0, 1}
Next == x' = x + 1 /\ Assert(TLCGet("distinct") < 4, <<"rebuilt", x>>)
Inv == x < 3 \/ 1 \in x
====
INIT Init
NEXT Next
INVARIANT Inv
CHECK_DEADLOCK FALSE
