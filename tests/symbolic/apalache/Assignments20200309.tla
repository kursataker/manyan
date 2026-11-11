\* options: -length 1
\* apalache-mc check --length=1 --inv=Inv Assignments20200309.tla: ERROR (255)
----- MODULE Assignments20200309 -----
VARIABLE
    \* @type: Int;
    a

\* this specification fails, as it has no expression
\* that can be treated as an assignment
Init == TRUE
Next == a' = a
Inv == FALSE
===============
INIT Init
NEXT Next
INVARIANT Inv
