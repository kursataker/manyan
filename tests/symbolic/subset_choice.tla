---- MODULE subset_choice ----
\* A step chooses any subset; one with three elements breaks Inv.
EXTENDS Integers, FiniteSets
VARIABLES
    \* @type: Set(Int);
    s
Init == s = {}
Next == s' \in SUBSET (1..4)
Inv == Cardinality(s) < 3
====
INIT Init
NEXT Next
INVARIANT Inv
