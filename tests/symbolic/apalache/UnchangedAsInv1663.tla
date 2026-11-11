\* options: -length 1
\* apalache-mc check --inv=Inv --length=1 UnchangedAsInv1663.tla: OK
------------------------- MODULE UnchangedAsInv1663 ---------------------------
EXTENDS Integers

VARIABLE
    \* @type: Int;
    x

Init ==
    x = 0

Next ==
    \/ x < 0 /\ x' = x + 1
    \/ x >= 0 /\ UNCHANGED x

Inv ==
    UNCHANGED x

===============================================================================
INIT Init
NEXT Next
INVARIANT Inv
