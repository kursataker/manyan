---- MODULE case_none ----
EXTENDS Naturals, TLC
ASSUME PrintT(CASE 1 = 2 -> 0 [] 2 = 3 -> 1)
====
