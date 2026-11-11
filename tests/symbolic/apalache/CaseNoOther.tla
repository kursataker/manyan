\* options: -length 10
\* apalache-mc check CaseNoOther.tla: OK
----------------------------- MODULE CaseNoOther ------------------------------
\* a regression test for issue #285
VARIABLE
    \* @type: Int;
    x

Init ==
    x = 0 \/ x = 1

Next ==
    x' = CASE x = 0 -> 1
           [] x = 1 -> 2
           [] x = 2 -> 0

===============================================================================
INIT Init
NEXT Next
