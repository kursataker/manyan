\* options: -length 1
\* apalache-mc check --inv=Inv --length=1 NoVars.tla: OK
---------- MODULE NoVars ----------

Init == TRUE
Next == TRUE
Inv == TRUE

====================
INIT Init
NEXT Next
INVARIANT Inv
