---- MODULE integer_too_big_assume ----
\* Definitions are checked before assumptions.
EXTENDS Integers, FiniteSets, TLC
ASSUME 2147483648 > 0
Unused == 2147483649
====
