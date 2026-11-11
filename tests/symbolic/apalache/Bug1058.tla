\* options: -length 10
\* apalache-mc check Bug1058.tla: OK
---- MODULE Bug1058 ----
EXTENDS Naturals

VARIABLE
  \* @type: Int -> Int;
  f

Init == 
  \/ f \in [{1} -> {}]
  \/ f \in [{} -> {}] 
  \/ f \in [{} -> {1}] 

Next == f' = f

====
INIT Init
NEXT Next
