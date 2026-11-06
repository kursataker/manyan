---- MODULE next_in_infinite ----
\* TLC enumerates the right side of x' \in S, failing for an infinite set.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' \in Nat
====
INIT Init
NEXT Next
