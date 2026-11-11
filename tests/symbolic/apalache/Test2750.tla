\* options: -length 10
\* apalache-mc check --config=Test2750.cfg Test2750.tla: OK
----------------------- MODULE Test2750 -----------------------------
VARIABLES
    \* @type: A;
    x

CONSTANT
    \* @type: A;
    C


Init ==
    /\ x = C

Next ==
    UNCHANGED x

Inv == TRUE

===============================================================================
INIT Init
NEXT Next
INVARIANT Inv
CONSTANT C = "1_OF_A"
