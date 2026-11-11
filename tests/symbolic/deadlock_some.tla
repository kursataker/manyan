---- MODULE deadlock_some ----
\* One behavior stops, another goes on: Apalache reports a deadlock only
\* when no behavior goes on.
EXTENDS Integers
VARIABLES
    \* @type: Int;
    x
Init == x \in {0, 10}
Next == (x < 2 /\ x' = x + 1) \/ (x >= 10 /\ x' = x)
Inv == x >= 0
====
INIT Init
NEXT Next
INVARIANT Inv
