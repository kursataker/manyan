---- MODULE nested ----
EXTENDS Naturals, TLC
---- MODULE Inner ----
EXTENDS Naturals
CONSTANT K
Twice == 2 * K
====
I == INSTANCE Inner WITH K <- 21
ASSUME PrintT(I!Twice)
====
