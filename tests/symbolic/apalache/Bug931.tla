\* options: -length 10
\* apalache-mc check --inv=Inv Bug931.tla: ERROR (255)
---------- MODULE Bug931 ----------
EXTENDS  FiniteSets

Init == TRUE
Next == TRUE
Inv == Cardinality({{}}) = 1

===================================
INIT Init
NEXT Next
INVARIANT Inv
