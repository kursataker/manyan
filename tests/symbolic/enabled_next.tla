---- MODULE enabled_next ----
\* ENABLED in an action, which symbolic checking refuses.
EXTENDS Naturals
VARIABLE
    \* @type: Int;
    x
Init == x = 0
Up == x < 2 /\ x' = x + 1
Next == (ENABLED Up /\ Up) \/ (~ENABLED Up /\ x' = 0)
Inv == x < 3
====
INIT Init
NEXT Next
INVARIANT Inv
