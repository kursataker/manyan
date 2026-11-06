---- MODULE constant_no_source ----
\* TLC computes a constant definition before the search, with no source:
\* enumerating its set of records fails with no value it cannot fingerprint.
EXTENDS Naturals
VARIABLE x
S == [a : Nat]
Init == x \in S
Next == UNCHANGED x
====
INIT Init
NEXT Next
