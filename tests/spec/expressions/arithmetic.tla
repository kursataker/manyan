---- MODULE arithmetic ----
EXTENDS Integers, TLC
ASSUME PrintT(<<1 + 2 * 3, (1 + 2) * 3, 2 ^ 10, 10 - 3 - 2, 7 \div 2, -7 \div 2, 7 % 3, -7 % 3>>)
ASSUME PrintT(<<1 < 2, 2 <= 2, 3 =< 2, 3 > 2, 2 >= 3, 2 \leq 2, 2 \geq 3, -(3), - -3>>)
ASSUME PrintT(<<(-2) ^ 3, 2147483647 - 1, -2147483647 - 1>>)
====
