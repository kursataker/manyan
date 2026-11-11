---- MODULE no_error ----
\* Two counters stay equal.
EXTENDS Integers
VARIABLES
    \* @type: Int;
    x,
    \* @type: Int;
    y
Init == x = 0 /\ y = 0
Next == \/ x < 100 /\ x' = x + 1 /\ y' = y + 1
        \/ x > 0 /\ x' = x - 1 /\ y' = y - 1
Inv == x = y
====
INIT Init
NEXT Next
INVARIANT Inv
