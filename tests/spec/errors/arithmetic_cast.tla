---- MODULE arithmetic_cast ----
\* Arithmetic on a non-integer is a Java cast error.
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT({1} + 1)
====
