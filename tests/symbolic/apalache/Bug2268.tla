\* options: -length 1 -cinit CInit
\* apalache-mc check --length=1 --cinit=CInit Bug2268.tla: OK
-------------------------- MODULE Bug2268 -----------------------------

CONSTANT
  \* @type: Set(X);
  N

CInit == 
  /\ N \in SUBSET {"1_OF_X"}
  /\ N /= {}

Init == TRUE
Next == TRUE  

=========================================================================
INIT Init
NEXT Next
