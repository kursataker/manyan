---- MODULE trace ----
\* TLCExt's Trace: the behavior to the state being checked, or to the one
\* whose successors are generated; each state a record of the variables,
\* which Json's Java writes in TLC's order of the variables.
EXTENDS TLC, TLCExt, Json, Naturals, Sequences
VARIABLES zz, b, a
Init == zz = 0 /\ b = [q |-> 1, p |-> 2] /\ a = "s"
Next == /\ zz < 2
        /\ PrintT(<<"Next", Len(Trace), Trace[Len(Trace)].zz>>)
        /\ zz' = zz + 1 /\ b' = b /\ a' = a
Inv == /\ PrintT(<<"Inv", Len(Trace)>>)
       /\ (zz < 2 \/ (PrintT(Trace) /\ PrintT(ToJson(Trace)) /\ PrintT(ToJsonObject(Trace[1])) /\ FALSE))
====
INIT Init
NEXT Next
INVARIANT Inv
