---- MODULE quantifier_numeral ----
\* A quantifier bound that is a number is named as written.
EXTENDS Integers, FiniteSets, TLC
ASSUME \E x \in 007 : TRUE
====
