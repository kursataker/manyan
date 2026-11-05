---- MODULE java_wrap ----
\* An error inside an operator TLC implements in Java carries its preamble.
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT(SelectSeq(<<1, 2>>, LAMBDA v : <<1>>[v] = 1))
====
