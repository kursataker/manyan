---- MODULE invariant_initial ----
\* An invariant violated by an initial state, which TLC prints apart.
EXTENDS Naturals
VARIABLE
    \* @type: Int;
    x
Init == x \in {0, 1}
Next == x' = x
Inv == x = 0
====
INIT Init
NEXT Next
INVARIANT Inv
