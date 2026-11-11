\* options: -length 10
\* apalache-mc check --inv=Inv ConfigReplacements.tla: ERROR (12)
----------------------- MODULE ConfigReplacements_2 -----------------------------
VARIABLES
    \* @type: Int;
    x

Value == 0

Value2 == 2

Init ==
    /\ x = Value

Next ==
    UNCHANGED x

Inv ==
    x = 2

===============================================================================
INIT Init
NEXT Next
INVARIANT Inv
