---- MODULE constant_operator_argument ----
\* A constant operator, and a definition of an instance, are operator
\* arguments by their bare names.
EXTENDS Naturals
CONSTANT Op(_)
VARIABLE s
I == INSTANCE constant_operator_argument_I
D(F(_), x) == F(x)
Test(e) == e < 5
Init == s = 0
Next == s' = (s + 1) % 3
Inv == D(Op, s) /\ D(I!Small, s)
====
CONSTANT Op <- Test
INIT Init
NEXT Next
INVARIANT Inv
