---- MODULE member_record ----
\* Membership in a record, which is not a set.
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT(1 \in [a |-> 1])
====
