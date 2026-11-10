---- MODULE tlcget_action ----
\* TLCGet("action") in a simulation: the action into the state, with its
\* arguments, as the invariants, the actions and the ALIAS see it; primed,
\* the next state's, and in the last state its own. Init's has no context.
\* options: -simulate num=1 -depth 6 -seed 1
EXTENDS Naturals, TLC
VARIABLE x, y
Init == x = 0 /\ y = 0
Inc(n, m) == x < 3 /\ PrintT(<<"next", x, TLCGet("action").name>>) /\ x' = x + n /\ y' = m
Reset == x = 3 /\ x' = 0 /\ y' = y
Next == \E n \in {1}, m \in {"a"} : Inc(n, m) \/ Reset
Inv == PrintT(<<"inv", x, TLCGet("action").name>>)
Bad == x # 2
Alias == [x |-> x, a |-> TLCGet("action"), b |-> TLCGet("action")']
====
INIT Init
NEXT Next
INVARIANT Inv Bad
ALIAS Alias
