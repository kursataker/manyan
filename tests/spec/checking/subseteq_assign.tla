---- MODULE subseteq_assign ----
\* x' \subseteq S takes each subset of S, as x' \in SUBSET S.
EXTENDS Naturals
VARIABLE x
Init == x = {}
Next == x' \subseteq {1, 2} /\ x' # {1}
====
INIT Init
NEXT Next
