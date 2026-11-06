---- MODULE constant_fairness_form ----
\* []<>c and <>[]c, joined at the top, TLC checks without its tableau, and
\* there a constant has its value: neither property is violated.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == UNCHANGED x
P == <>[]TRUE
Q == []<>TRUE \/ <>[](x = 1)
====
INIT Init
NEXT Next
PROPERTY P Q
