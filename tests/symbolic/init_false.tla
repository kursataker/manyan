---- MODULE init_false ----
\* No initial state.
EXTENDS Integers
VARIABLES
    \* @type: Int;
    x
Init == x \in 1..3 /\ x > 5
Next == x' = x
Inv == x > 5
====
INIT Init
NEXT Next
INVARIANT Inv
