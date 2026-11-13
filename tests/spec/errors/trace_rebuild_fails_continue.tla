---- MODULE trace_rebuild_fails_continue ----
\* options: -continue
\* With -continue an invariant failing to evaluate is an error of the step,
\* and TLC's replay of the step fails too.
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
