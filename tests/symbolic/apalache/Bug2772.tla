\* options: -length 1
\* apalache-mc check --inv=OkInv --length=1 Bug2772.tla: OK
---- MODULE Bug2772 ----
EXTENDS Integers, Apalache

ErrInv == [x \in {0} |-> 1] \in UNION { [d -> {1}] : d \in {{0}} }

OkInv == \E d \in {{0}}: [x \in {0} |-> 1] \in [d -> {1}]

Init == TRUE

Next == TRUE
====
INIT Init
NEXT Next
INVARIANT OkInv
