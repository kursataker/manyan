---- MODULE seq_member_initial ----
\* A value that is not a sequence checked against Seq(S) is Sequences'
\* error, which TLC reports without the initial state (miguelmota's
\* AlternatingBit, whose channels start as {}).
EXTENDS Sequences
VARIABLE ch
Init == ch = {}
Next == UNCHANGED ch
TypeOK == ch \in Seq({0, 1})
====
INIT Init
NEXT Next
INVARIANT TypeOK
