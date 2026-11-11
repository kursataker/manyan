\* options: -length 1
\* apalache-mc check --length=1 ExistTuple476.tla: OK
------------------ MODULE ExistTuple476 ----------------------
(* A regression test for the issue:
   https://github.com/apalache-mc/apalache/issues/476
 *)
EXTENDS Integers

VARIABLES
    \* @type: Int;
    x,
    \* @type: Int;
    y

Init ==
    x = 0 /\ y = 0

Next ==
    \E <<i, j>> \in (1..2) \X (3..4):
        /\ x' = i
        /\ y' = j

==============================================================
INIT Init
NEXT Next
