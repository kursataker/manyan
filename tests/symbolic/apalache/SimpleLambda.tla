\* options: -length 10
\* apalache-mc check --inv=Inv SimpleLambda.tla: OK
--------------------- MODULE SimpleLambda -----------------                      
L(F(_), n) == F(n)

Init ==
    TRUE

Next ==
    TRUE

Inv ==
    L(LAMBDA i: i, 3) = 3

===========================================================
INIT Init
NEXT Next
INVARIANT Inv
