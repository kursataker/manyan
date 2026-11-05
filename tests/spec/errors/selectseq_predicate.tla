---- MODULE selectseq_predicate ----
\* SelectSeq needs a Boolean from its test.
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT(SelectSeq(<<1>>, LAMBDA v : 2))
====
