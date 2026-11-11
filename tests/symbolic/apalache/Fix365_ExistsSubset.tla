\* options: -length 10
\* apalache-mc check --length=10 Fix365_ExistsSubset.tla: OK
--------------------- MODULE Fix365_ExistsSubset ------------------------------
EXTENDS Integers, FiniteSets

VARIABLE
    \* @type: Set(Str);
    S

Init ==
    S = {"a", "b", "c"}

Next ==
    \E X \in SUBSET S:
        /\ Cardinality(X) >= 2
        /\ S' = X

===============================================================================
INIT Init
NEXT Next
