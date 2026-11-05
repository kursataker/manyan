---- MODULE reals_set ----
\* Real is the string "Reals" in TLC's Reals module.
EXTENDS Reals, TLC
ASSUME PrintT(Real)
ASSUME PrintT(3 \in Real)
====
