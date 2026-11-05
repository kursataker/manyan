---- MODULE member_seq ----
\* Only a sequence or a string may be in Seq(S).
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT(<<1>> \in Seq({1}))
ASSUME PrintT((2 :> 1) \in Seq({1}))
====
