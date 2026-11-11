\* options: -length 1
\* apalache-mc check --inv=Inv --length=1 Bug1735.tla: ERROR (12)
---- MODULE Bug1735 ----

EXTENDS Integers

Init == TRUE
Next == TRUE

Inv ==
    LET f == [ x \in {2} |-> 2 ] IN
    LET S == [{ 1, 2 } -> { 2 }] IN
    f \in S

=================
INIT Init
NEXT Next
INVARIANT Inv
