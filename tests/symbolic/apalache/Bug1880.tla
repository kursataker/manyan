\* options: -length 1
\* apalache-mc check --length=1 Bug1880.tla: OK
-------------------------------- MODULE Bug1880 --------------------------------
EXTENDS FiniteSets

VARIABLES
    \* @type: Set(Int);
    set

Init ==
    LET \* @type: () => Set(a);
        Empty == {}
    IN
    set = Empty

Next == UNCHANGED set
===============================================================================
INIT Init
NEXT Next
