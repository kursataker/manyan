---- MODULE cardinality_overflow ----
\* A set too large to count: TLC's counts are 32-bit ints.
EXTENDS Naturals, FiniteSets
ASSUME Cardinality((1..2000) \X (1..2000) \X (1..2000)) > 0
====
