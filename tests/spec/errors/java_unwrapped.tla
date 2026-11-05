---- MODULE java_unwrapped ----
\* An error TLC raises as an EvalException is not wrapped.
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT(SelectSeq(<<1>>, LAMBDA v : v \div 0 = 1))
====
