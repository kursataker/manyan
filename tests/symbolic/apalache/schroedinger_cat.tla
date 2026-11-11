\* options: -length 10
\* apalache-mc check --run-dir=./profile-run-dir --smtprof schroedinger_cat.tla: OK
------------------------- MODULE schroedinger_cat ----------------------------
VARIABLE
    \* @type: Str;
    state

Init == state = "ALIVE"
Next == state' = state \/ state' = "DEAD"
Inv == state = "ALIVE"
==============================================================================
INIT Init
NEXT Next
