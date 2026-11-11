\* options: -length 1
\* apalache-mc check --inv=Inv --length=1 FoldSeqSeq.tla: OK
------------------------ MODULE FoldSeqSeq ------------------------------------
EXTENDS Apalache, Integers, Sequences

VARIABLE 
\* @type: Int;
x

\* @type: (Seq(Int), Int) => Seq(Int);
A(p,q) == Append(p,q)

Init == x = 0
Next == UNCHANGED x
Inv == ApaFoldSeqLeft( A, <<>>, <<1,2,3>> ) = <<1,2,3>>
===============================================================================
INIT Init
NEXT Next
INVARIANT Inv
