---- MODULE trace_alias_simulate ----
\* options: -simulate num=1 -depth 23 -seed 107
\* In a simulation too, an ALIAS sees Trace as the behavior to the state it
\* prints.
EXTENDS TLC, TLCExt, Json, Naturals, Sequences
VARIABLE x
Init == x = 0
Next == x' = x + 1
Inv == x < 3
Alias == [x |-> x, n |-> Len(Trace), json |-> ToJsonObject(Trace[Len(Trace)])]
====
INIT Init
NEXT Next
INVARIANT Inv
ALIAS Alias
