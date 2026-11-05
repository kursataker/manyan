---- MODULE subseteq_infinite ----
\* Nat cannot be enumerated to test S \subseteq T.
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT(Nat \subseteq {1})
====
