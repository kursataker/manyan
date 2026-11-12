\* options: -length 10
\* apalache-mc check ConfigUnsorted.tla: ERROR (255)
-------------------------- MODULE ConfigUnsorted ----------------------------------------
(* A specification that introduces preprocessing issues *)
VARIABLES
    \* @type: Int;
    x

A == 1
B == 2
C == 3

\* the following annotations introduce a circular dependency
OVERRIDE_A == B
OVERRIDE_B == C
OVERRIDE_C == A

Init == x = 0
Next == UNCHANGED x
========================================================================================
INIT Init
NEXT Next
