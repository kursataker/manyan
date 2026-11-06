---- MODULE instance_operator ----
\* A constant operator is substituted implicitly by the operator of its
\* name, here itself a constant operator an outer instance substitutes.
EXTENDS Naturals, TLC
F(n) == n + 1
INSTANCE Mid
ASSUME PrintT(<<Twice(1), Thrice(1)>>)
====
