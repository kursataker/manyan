\* options: -length 10
\* apalache-mc check --inv=Inv Slicer1.tla: ERROR (12)
--------------------------- MODULE Slicer1 ------------------------------------
(* Testing slicing of symbolic transitions *)

VARIABLE
    \* @type: Str;
    state

Init ==
    state = "Init"

A ==
    state' = "A"

B ==
    state' = "B"

Next ==
    A \/ B

Inv ==
    state /= "B"
===============================================================================
INIT Init
NEXT Next
INVARIANT Inv
