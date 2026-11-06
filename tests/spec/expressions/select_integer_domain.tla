---- MODULE select_integer_domain ----
\* A field of a function TLC compares the domain of with it, in order.
EXTENDS TLC
ASSUME PrintT((1 :> 2).f)
====
