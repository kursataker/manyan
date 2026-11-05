---- MODULE infinite_function_equality ----
\* Comparing functions over Nat computes them, which TLC cannot.
EXTENDS Naturals, TLC
ASSUME PrintT([x \in Nat |-> x] = [x \in Nat |-> x])
====
