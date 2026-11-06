---- MODULE tlcget_counters ----
\* TLCGet's counters of the search: a new state is distinct as it is
\* checked, and joins the queue then, an initial one before.
EXTENDS Naturals, TLC
VARIABLE x
S == <<TLCGet("distinct"), TLCGet("queue"), TLCGet("diameter"), TLCGet("duration") >= 0>>
ASSUME PrintT(<<"assume", S>>)
Init == x \in {0, 1} /\ PrintT(<<"init", x, S>>)
Next == x < 4 /\ PrintT(<<"next", x, S>>) /\ x' \in {x + 1, x + 2}
Inv == PrintT(<<"inv", x, S>>)
Prop == [][PrintT(<<"step", x, x', S>>)]_x
====
INIT Init
NEXT Next
INVARIANT Inv
PROPERTY Prop
CHECK_DEADLOCK FALSE
