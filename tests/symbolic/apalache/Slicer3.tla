\* options: -length 10
\* apalache-mc check --inv=Inv Slicer3.tla: ERROR (12)
--------------------------- MODULE Slicer3 ------------------------------------
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
        LET C == { 4, 5, 6 }
        IN
        state' = "B"

Next ==
    A \/ B

Inv ==
    state /= "B"
===============================================================================
INIT Init
NEXT Next
INVARIANT Inv
