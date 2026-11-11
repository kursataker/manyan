---- MODULE sets ----
\* A set grows by an element a step; Cardinality sees the duplicates.
EXTENDS Integers, FiniteSets
VARIABLES
    \* @type: Set(Int);
    s
Init == s = {}
Next == \E v \in 1..3 : s' = s \cup {v}
Inv == Cardinality(s) < 3
====
INIT Init
NEXT Next
INVARIANT Inv
