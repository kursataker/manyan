\* options: -length 10
\* apalache-mc check --inv=Inv --discard-disabled=false TrivialFail.tla: ERROR (12)
------ MODULE TrivialFail ------
(* A spec which can always advance but with an invariant that always fails.

   Introduced to address https://github.com/apalache-mc/apalache/issues/2158 *)

Init == TRUE
Next == TRUE
Inv == FALSE
================================
INIT Init
NEXT Next
INVARIANT Inv
