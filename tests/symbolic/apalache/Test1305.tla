\* options: -length 10 -max-error 10
\* apalache-mc check --inv=Inv --view=View --max-error=10 Test1305.tla: ERROR (12)
------------------------------ MODULE Test1305 --------------------------------
\* A regression test for #1305:
\* Refactoring of TLC operators broke the support for --view on functions
EXTENDS Integers

VARIABLE
    \* @type: Int -> Int;
    fun

Init ==
    fun = [ x \in 1..2 |-> x ]

Next ==
    \/ UNCHANGED fun
    \/ fun' = [ x \in DOMAIN fun |-> fun[x] + x ]

Inv ==
    fun[1] /= 2

View == fun

===============================================================================
INIT Init
NEXT Next
INVARIANT Inv
VIEW View
