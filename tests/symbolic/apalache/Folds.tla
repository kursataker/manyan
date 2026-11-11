\* options: -length 1
\* apalache-mc check --inv=Inv --length=1 Folds.tla: OK
------------------------ MODULE Folds ------------------------------------
EXTENDS Integers, Apalache

VARIABLE 
\* @type: Int;
 x

Init == x = 1

Op(p,q) == p + q

Next == UNCHANGED x

Inv == ApaFoldSet( Op, 0, {1,2,3} ) = 6 /\ ApaFoldSeqLeft( Op, 99, <<>> ) = 99
===============================================================================
INIT Init
NEXT Next
INVARIANT Inv
