---- MODULE identifiers ----
EXTENDS Naturals, TLC
x1 == 1
_y == 2
1a == 3
a_b_C == 4
ASSUME PrintT(<<x1, _y, 1a, a_b_C>>)
====
