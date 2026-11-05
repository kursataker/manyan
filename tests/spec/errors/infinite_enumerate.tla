---- MODULE infinite_enumerate ----
\* Enumerating a function set with an infinite range.
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT([{1} -> Nat] = {})
====
