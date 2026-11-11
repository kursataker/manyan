\* options: -length 10
\* apalache-mc check Callback.tla: OK
----------------------------- MODULE Callback ----------------------------
EXTENDS Integers

VARIABLES
    \* @type: Int;
    x

Pick(Cb(_)) ==
    \E i \in 1..10:
        Cb(i)

Init ==
    LET Cb(i) ==
        x = i
    IN
    Pick(Cb)

Next ==
    LET Cb(i) ==
        x' = i
    IN
    Pick(Cb)

Inv ==
    x > 0
==========================================================================
INIT Init
NEXT Next
