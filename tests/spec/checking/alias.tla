---- MODULE alias ----
\* With an ALIAS, each state of a trace prints as the alias's value on the
\* step to the next state (the last state's to itself), its fields in the
\* constructor's order, at the state's level. An error evaluating it is
\* printed as a variable _ALIASEvalError after the state's.
EXTENDS Naturals, TLC
VARIABLE x, y
Init == x = 0 /\ y = 0
Next == x < 3 /\ x' = x + 1 /\ y' = y
Alias == [double |-> 2 * x, x |-> x, next |-> x', level |-> TLCGet("level"), bad |-> 1 \div (x - 1)]
====
INIT Init
NEXT Next
ALIAS Alias
