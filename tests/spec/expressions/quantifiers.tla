---- MODULE quantifiers ----
EXTENDS Naturals, TLC
S == 1..4
ASSUME PrintT(<<\A x \in S : x > 0, \E x \in S : x > 3, \E x \in S : x > 4>>)
ASSUME PrintT(<<\A x, y \in S : x + y > 1, \E x \in S, y \in {10} : x + y = 13>>)
ASSUME PrintT(<<\E <<a, b>> \in S \X S : a * b = 12, \A x \in {} : FALSE, \E x \in {} : TRUE>>)
====
