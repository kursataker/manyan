---- MODULE partition ----
\* Partitions of 1..N, refined and coarsened a block at a time: sets of
\* sets, SUBSET and UNION.
EXTENDS Naturals, FiniteSets
CONSTANT N
VARIABLE blocks
Init == blocks = {1..N}
Split == \E b \in blocks : \E s \in SUBSET b :
           s # {} /\ s # b /\ blocks' = (blocks \ {b}) \cup {s, b \ s}
Merge == \E a, b \in blocks : a # b /\ blocks' = (blocks \ {a, b}) \cup {a \cup b}
Next == Split \/ Merge
Covers == UNION blocks = 1..N
Disjoint == \A a, b \in blocks : a = b \/ a \cap b = {}
====
CONSTANT N = 9
INIT Init
NEXT Next
INVARIANT Covers
INVARIANT Disjoint
