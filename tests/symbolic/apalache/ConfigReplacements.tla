\* options: -length 10
\* apalache-mc check --config=ConfigReplacements2.cfg ConfigReplacements.tla: OK
----------------------- MODULE ConfigReplacements -----------------------------
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
CONSTANTS Value <- Value2
