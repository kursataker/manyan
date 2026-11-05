---- MODULE subseq_domain ----
\* SubSeq names the argument outside the domain: here the third.
EXTENDS Sequences, TLC
ASSUME PrintT(SubSeq(<<1, 2>>, 2, 1))
ASSUME PrintT(SubSeq(<<1, 2>>, 1, 5))
====
