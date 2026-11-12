---- MODULE tlc_defer_actions ----
\* A TLCDefer met in an action with no successor is dropped: the simulation
\* tries the next action afresh. Within one, the last met is kept, from a
\* branch with no successor too.
\* options: -simulate num=2 -depth 5 -seed 6
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x = 0
A == TLCDefer(PrintT(<<"A", x>>)) /\ x > 100 /\ x' = x
B == \E i \in {1, 2} : TLCDefer(PrintT(<<"B", x, i>>)) /\ i = 2 /\ x' = x + i
C == x' = x + 1
Next == A \/ B \/ C
====
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
