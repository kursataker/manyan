---- MODULE infinite_equality ----
\* TLC compares Nat with no other set.
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT(Nat = Nat)
ASSUME PrintT(Nat = {1})
====
