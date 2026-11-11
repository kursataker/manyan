\* options: -length 3
\* apalache-mc check --length=3 --config=Test1623.cfg --inv=Inv Test1623.tla: OK
------------------ MODULE Test1623 ---------------------------            
EXTENDS Integers

CONSTANT
    \* @type: Int;
    N

ASSUME (N \in Nat)

VARIABLE
    \* @type: Int;
    x

Init ==
    x = N

Next ==
    \/ x > 0 /\ x' = x - 1
    \/ x = 0 /\ UNCHANGED x

Inv ==
    x >= 0
=====================================================================
INIT Init
NEXT Next
CONSTANTS N = 4
INVARIANT Inv
