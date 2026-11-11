\* options: -length 0
\* apalache-mc check --inv=Inv --length=0 RepeatBad.tla: ERROR (255)
------------------------ MODULE RepeatBad ------------------------------------
EXTENDS Apalache, Integers

Inv ==
    LET Op(a, i) == a + 1
    \* The 2nd argument to Repeat must be an integer literal
    IN \E x \in {5} : Repeat(Op, x, 0) = 5

Init == TRUE
Next == TRUE
===============================================================================
INIT Init
NEXT Next
INVARIANT Inv
