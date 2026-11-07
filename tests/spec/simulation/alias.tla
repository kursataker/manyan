---- MODULE alias ----
\* A simulated behavior prints as its alias.
\* options: -simulate num=1 -depth 5 -seed 1
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x' = x + 1
Inv == x < 3
Alias == [z |-> x, n |-> x', l |-> TLCGet("level")]
====
INIT Init
NEXT Next
INVARIANT Inv
ALIAS Alias
