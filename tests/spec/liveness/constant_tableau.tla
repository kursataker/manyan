---- MODULE constant_tableau ----
\* In TLC's tableau a constant is never evaluated: the negation of
\* <>TRUE \/ <>(x = 1) is []FALSE /\ [](x # 1), and its FALSE holds.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == UNCHANGED x
P == <>TRUE \/ <>(x = 1)
====
INIT Init
NEXT Next
PROPERTY P
