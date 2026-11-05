---- MODULE union_infinite ----
\* UNION enumerates each of its sets.
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT(UNION {{1}, Nat} = {})
====
