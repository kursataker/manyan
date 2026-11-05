---- MODULE instance_incomplete ----
\* An instance's name is not an expression without an operator after it.
EXTENDS Naturals
I == INSTANCE Naturals
ASSUME I = 1
====
