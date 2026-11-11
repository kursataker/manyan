\* options: -length 2
\* apalache-mc check --length=2 --inv=Inv Bug20200306.tla: ERROR (12)
----- MODULE Bug20200306 -----
VARIABLE
    \* @type: Int;
    a
Init == a = 1
Next == a' = a
Inv == FALSE
===============
INIT Init
NEXT Next
INVARIANT Inv
