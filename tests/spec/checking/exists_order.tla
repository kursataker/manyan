---- MODULE exists_order ----
\* \E i, j \in S enumerates its bindings with the first variable varying
\* fastest, as TLC: in the initial states, and in the successors.
EXTENDS Naturals
VARIABLE x
Init == \E i, j \in 1..2 : i # j /\ x = <<i, j>>
Next == \E i, j \in 1..3 : x' = <<x[1] + i, j>>
Inv == x[1] < 4 \/ x[2] = 1
====
INIT Init
NEXT Next
INVARIANT Inv
