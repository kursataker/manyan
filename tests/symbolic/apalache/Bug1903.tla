\* options: -length 1
\* apalache-mc check --length=1 Bug1903.tla: OK
-------------------------- MODULE Bug1903 -----------------------------

F(a,b) == a /= b
G(a,b) == F(b,a) 

Init == TRUE

Next == TRUE

Inv == G(1,2)

=========================================================================
INIT Init
NEXT Next
