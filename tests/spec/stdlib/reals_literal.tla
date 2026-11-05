---- MODULE reals_literal ----
\* TLC cannot evaluate a decimal number.
EXTENDS Reals, TLC
ASSUME PrintT(1.5)
====
