---- MODULE initial_none ----
\* No initial state.
\* options: -simulate -depth 10 -seed 1
EXTENDS Integers, Sequences, TLC
VARIABLE x
Init == x = 0 /\ x = 1
Next == x' = x
====
INIT Init
NEXT Next
