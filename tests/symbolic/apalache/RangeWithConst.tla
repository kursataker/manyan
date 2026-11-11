\* options: -length 1 -cinit CInit
\* apalache-mc check --cinit=CInit --inv=Inv --length=1 RangeWithConst.tla: OK
------------------------ MODULE RangeWithConst ------------------------------------
EXTENDS Integers

VARIABLE 
\* @type: Int;
x

CONSTANT
\* @type: Int;
C

CInit == C = 1

Init == x = C
Next == UNCHANGED x
Inv == \E y \in (C + 1) .. 4 : y = 3
===============================================================================
INIT Init
NEXT Next
INVARIANT Inv
