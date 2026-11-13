---- MODULE trace_rebuild_violation_continue ----
\* options: -continue
\* With -continue the error rebuilding the behavior fails the invariant and
\* the step, and TLC's replay of the step fails too.
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
