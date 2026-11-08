---- MODULE split_let_call ----
\* A call of a LET definition is split as any other, its actions named by
\* the definition with its arguments.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == LET Go(p) == x' = x + p \/ x' = x + p + 1 IN Go(1)
Inv == x < 3
====
INIT Init
NEXT Next
INVARIANT Inv
