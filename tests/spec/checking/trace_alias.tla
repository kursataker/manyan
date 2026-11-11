---- MODULE trace_alias ----
\* An ALIAS sees Trace as TLC rebuilds the behavior to print it: the
\* behavior to the state it prints.
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
