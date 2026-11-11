---- MODULE counter_short ----
\* The violation is two steps away: one step finds none.
\* options: -length 1
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
