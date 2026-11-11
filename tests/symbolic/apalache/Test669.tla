\* options: -length 10
\* apalache-mc check Test669.tla: ERROR (255)
------------------------------ MODULE Test669 -------------------------------
EXTENDS Integers

CONSTANT
    \* @type: Int;
    N

VARIABLE
    \* @type: Int;
    x

Init == x = N
Next == x' = x - 1
=============================================================================
INIT Init
NEXT Next
