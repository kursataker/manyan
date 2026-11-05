---- MODULE reals_divide ----
\* Reals has dummy definitions in TLC: / chooses from the string "Reals".
EXTENDS Reals, TLC
ASSUME PrintT(1 / 2)
====
