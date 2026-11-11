---- MODULE trace ----
\* In a simulation TLCExt's Trace is the behavior so far, the successor
\* being checked last.
\* options: -simulate num=2 -depth 3 -seed 1
EXTENDS TLC, TLCExt, Naturals, Sequences
VARIABLE x
Init == x \in {0, 10}
Next == /\ PrintT(<<"Next", Trace>>)
        /\ x' = x + 1
Inv == PrintT(<<"Inv", [i \in 1..Len(Trace) |-> Trace[i].x]>>)
====
INIT Init
NEXT Next
INVARIANT Inv
