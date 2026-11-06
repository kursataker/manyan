---- MODULE equality_messages ----
\* TLC's errors comparing a boolean, a record, and Nat with other values.
EXTENDS Naturals, TLC
ASSUME PrintT([a |-> 1] = ("a" :> 1))
ASSUME PrintT([a |-> 1] = <<>>)
ASSUME PrintT(TRUE = "f")
====
