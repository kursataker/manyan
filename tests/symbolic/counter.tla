---- MODULE counter ----
\* A counter from 0..2 reaches 4 in two steps.
EXTENDS Integers
VARIABLES
    \* @type: Int;
    x
Init == x \in 0..2
Next == x < 5 /\ x' = x + 1
Inv == x /= 4
====
INIT Init
NEXT Next
INVARIANT Inv
