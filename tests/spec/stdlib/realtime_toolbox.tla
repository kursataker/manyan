---- MODULE realtime_toolbox ----
\* RealTime and Toolbox are standard modules: RealTime declares now, and
\* Toolbox's _TEPosition is the level of the state an alias shows.
EXTENDS Naturals, RealTime, Toolbox
VARIABLE x
Init == x = 0 /\ now = 0
Next == x' = x + 1 /\ now' = now
Alias == [x |-> x, pos |-> _TEPosition]
Small == x < 2
====
INIT Init
NEXT Next
ALIAS Alias
INVARIANT Small
