---- MODULE java_wrap_nested ----
\* Each enclosing Java operator wraps the error again.
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT(SelectSeq(<<1>>, LAMBDA v : (v + TRUE) = 1))
====
