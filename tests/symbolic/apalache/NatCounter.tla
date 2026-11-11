\* options: -length 10
\* apalache-mc check --length=10 --inv=Inv NatCounter.tla: ERROR (12)
----------------------------- MODULE NatCounter ------------------------
EXTENDS Naturals

VARIABLE
    \* @type: Int;
    x

Init == x = 3

\* a natural counter can go below zero, and this is expected behavior
Next == x' = x - 1

Inv == x >= 0
========================================================================
INIT Init
NEXT Next
INVARIANT Inv
