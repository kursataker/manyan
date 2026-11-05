---- MODULE initial_incomplete ----
\* An initial state that leaves a variable out.
\* options: -simulate -depth 10 -seed 1
EXTENDS Integers, Sequences, TLC
VARIABLES x, y
Init == x = 0
Next == x' = x /\ y' = y
====
INIT Init
NEXT Next
