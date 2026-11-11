\* options: -length 10
\* apalache-mc check CaseNoOtherBool.tla: OK
-------------------------- MODULE CaseNoOtherBool ------------------------------
\* a regression test for issue #285
VARIABLE
    \* @type: Int;
    x

Init ==
    x = 0 \/ x = 1

Next ==
    CASE x = 0 -> x' = 1
      [] x = 1 -> x' = 2
      [] x = 2 -> x' = 0

===============================================================================
INIT Init
NEXT Next
