---- MODULE lazy_args ----
EXTENDS Naturals, TLC
First(a, b) == a
Guard(c, e) == IF c THEN e ELSE 0
ASSUME PrintT(<<First(1, 1 \div 0), Guard(FALSE, CHOOSE x \in {} : TRUE)>>)
====
