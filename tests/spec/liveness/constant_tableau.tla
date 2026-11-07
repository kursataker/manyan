---- MODULE constant_tableau ----
\* A constant in the tableau is its value: the negation of
\* <>TRUE \/ <>(x = 1) is []FALSE /\ [](x # 1), which no behavior satisfies.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == UNCHANGED x
P == <>TRUE \/ <>(x = 1)
====
INIT Init
NEXT Next
PROPERTY P
