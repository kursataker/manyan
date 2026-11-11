\* options: -length 2
\* apalache-mc check --length=2 Test1343.tla: OK
-------------------- MODULE Test1343 ---------------------------

VARIABLES   
  \* @type: Int;
  x

Init ==  x = 0

Next == 
  \/ /\ \E t \in {0}:
       /\ FALSE
       /\ x' = 1
     /\ x' = 42
  \/ UNCHANGED x

=============================================================================    
INIT Init
NEXT Next
