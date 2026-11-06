---- MODULE compare_set_overridden ----
\* Sorting a set's elements, TLC compares each with the one before it: a
\* set compared with Nat cannot enumerate it.
EXTENDS Integers, TLC
ASSUME PrintT(3 \in UNION {Nat, {-1}})
====
