\* options: -length 10
\* apalache-mc check --length=10 Fix365_ExistsSubset2.tla: OK
---------------------- MODULE Fix365_ExistsSubset2 ------------------------------
EXTENDS Integers, FiniteSets

Rounds == 0..2

VARIABLE
    \* @type: Int -> Set(Str);
    msgs

Init ==
    msgs \in [Rounds -> SUBSET {"a", "b", "c"}]

Next ==
    \E X \in (SUBSET msgs[2]):
        /\ Cardinality(X) >= 2
        /\ msgs' = [msgs EXCEPT ![2] = X]

===============================================================================
INIT Init
NEXT Next
