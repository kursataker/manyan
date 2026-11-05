---- MODULE initial_error ----
\* An error computing the initial states.
\* options: -simulate -depth 10 -seed 1
EXTENDS Integers, Sequences, TLC
VARIABLE x
Init == x = CHOOSE y \in {1} : y > 1
Next == x' = x
====
INIT Init
NEXT Next
