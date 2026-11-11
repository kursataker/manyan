\* options: -length 0
\* apalache-mc check --out-dir=./test-out-dir --length=0 --debug Counter.tla: OK
------ MODULE Counter_2 ------
EXTENDS Naturals

VARIABLES
    \* @type: Int;
    x

Init == x \in 1..1000

Next ==
    \/ x \in 1..3000 /\ x' \in x..(x + 1000)
    \/ x \in 3000..1000000 /\ x' \in x..(x + 100000)

Inv == x /= 450000

============
INIT Init
NEXT Next
