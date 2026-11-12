\* options: -length 1
\* apalache-mc check --inv=ErrInv --length=1 Bug2772.tla: ERROR (255)
---- MODULE Bug2772_2 ----
EXTENDS Integers, Apalache

ErrInv == [x \in {0} |-> 1] \in UNION { [d -> {1}] : d \in {{0}} }

OkInv == \E d \in {{0}}: [x \in {0} |-> 1] \in [d -> {1}]

Init == TRUE

Next == TRUE
====
INIT Init
NEXT Next
INVARIANT ErrInv
