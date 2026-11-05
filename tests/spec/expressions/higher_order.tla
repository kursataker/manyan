---- MODULE higher_order ----
EXTENDS Naturals, TLC
Apply(F(_), x) == F(x)
Fold(Op(_, _), a, b, c) == Op(Op(a, b), c)
Inc(n) == n + 1
Map(F(_), S) == {F(x) : x \in S}
ASSUME PrintT(<<Apply(Inc, 1), Fold(+, 1, 2, 3), Apply(LAMBDA y : y * y, 5), Map(LAMBDA z : z + 10, {1, 2})>>)
ASSUME PrintT(Fold(LAMBDA a, b : a * b, 2, 3, 4))
====
