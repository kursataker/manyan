---- MODULE subset_filter_large ----
\* As TLC, a filter of SUBSET S takes each subset in turn: S may have more
\* elements than SUBSET S could hold.
EXTENDS Integers, FiniteSets, TLC
VARIABLE x
ASSUME PrintT(Cardinality({E \in SUBSET (1..25) : 1 \notin E /\ Cardinality(E) >= 23}))
Init == x \in {E \in SUBSET (1..25) : Cardinality(E) <= 1 /\ 25 \in E}
Next == UNCHANGED x
====
INIT Init
NEXT Next
