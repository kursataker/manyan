---- MODULE operators_user ----
EXTENDS Naturals, TLC
a ++ b == a * 10 + b
a ** b == a - b
-. a == 0 - a
a ^+ == a + 100
ASSUME PrintT(<<1 ++ 2 ++ 3, 5 ** 2, - 4, 3^+>>)
====
