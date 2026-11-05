---- MODULE assume_named ----
EXTENDS Naturals, TLC
ASSUME Positive == 1 > 0
ASSUMPTION PrintT("ok")
AXIOM TRUE
THEOREM T == 1 + 1 = 2
====
