---- MODULE deadlock_all ----
\* Every behavior stops after two steps.
EXTENDS Integers
VARIABLES
    \* @type: Int;
    x
Init == x = 0
Next == x < 2 /\ x' = x + 1
Inv == x >= 0
====
INIT Init
NEXT Next
INVARIANT Inv
