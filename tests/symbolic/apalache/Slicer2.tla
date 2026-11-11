\* options: -length 10
\* apalache-mc check --inv=Inv Slicer2.tla: ERROR (12)
--------------------------- MODULE Slicer2 ------------------------------------
(* Testing slicing of symbolic transitions *)

VARIABLE
    \* @type: Str;
    state

Init ==
    state = "Init"

A ==
    state' = "A"

B ==
    \E x \in {1, 2, 3}:
        state' = "B"

Next ==
    A \/ B

Inv ==
    state /= "B"
===============================================================================
INIT Init
NEXT Next
INVARIANT Inv
