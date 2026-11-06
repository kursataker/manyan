---- MODULE recursive_deep ----
\* Recursion hundreds of calls deep.
EXTENDS Naturals, TLC
RECURSIVE Sum(_)
Sum(n) == IF n = 0 THEN 0 ELSE n + Sum(n - 1)
f[n \in Nat] == IF n = 0 THEN 0 ELSE 1 + f[n - 1]
ASSUME PrintT(<<Sum(400), f[400]>>)
====
