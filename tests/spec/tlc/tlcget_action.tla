---- MODULE tlcget_action ----
\* TLCGet("action") outside a simulation: TLC keeps no action in a state,
\* and gives its UnnamedAction.
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x < 2 /\ x' = x + 1
Inv == PrintT(TLCGet("action").name)
Bad == x # 1
Alias == [x |-> x, a |-> TLCGet("action")]
====
INIT Init
NEXT Next
INVARIANT Inv Bad
ALIAS Alias
