\* options: -length 10
\* apalache-mc check --inv=Inv ExistsAsValue.tla: OK
------------------------ MODULE ExistsAsValue -------------------------
\* a test for the issue #148
VARIABLES
    \* @type: Bool;
    x

Init ==
    x = TRUE

Next ==
    x' = \E y \in {1, 2}: y /= 1

Next2 ==
    IF \E y \in {1, 2}: y /= 1
    THEN x' = TRUE
    ELSE x' = FALSE

Inv == x
=======================================================================
INIT Init
NEXT Next
INVARIANT Inv
