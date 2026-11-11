\* options: -length 2
\* apalache-mc check --length=2 --init=Init --next=Next --inv=Inv Fix333.tla: OK
------------------------- MODULE Fix333 -----------------------------------
VARIABLES
    \* @type: Bool;
    error

Init ==
    error = FALSE

Next ==
    error' = FALSE

Inv ==
    ~(error = TRUE)
===========================================================================
INIT Init
NEXT Next
INVARIANT Inv
