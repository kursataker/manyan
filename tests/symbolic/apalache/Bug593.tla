\* options: -length 10
\* apalache-mc check Bug593.tla: ERROR (255)
------------------------------- MODULE Bug593 -------------------------------
\* Module used in https://github.com/apalache-mc/apalache/issues/593
\* Test to see issue has been resolved

EXTENDS Integers, Bug593Aux

VARIABLE 
\* @type: { type: Str };
record

Init == record = [ type |-> "B" ]

Next ==
    LET result == Dummy IN
    UNCHANGED record
===============================================================================
INIT Init
NEXT Next
