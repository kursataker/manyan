\* options: -length 10
\* apalache-mc check --inv=Inv InlineAssumptions3318.tla: OK
----------------------- MODULE InlineAssumptions3318 -------------------------
(*
  A regression test for the issue identified in:
  https://github.com/apalache-mc/apalache/issues/3318
*)
EXTENDS Integers

VARIABLE
  \* @type: Int;
  x

ASSUME NamedAssumption == 1 = 1

Init == /\ x = 0
        /\ NamedAssumption

Next == UNCHANGED x

Inv == x = 0

================================================================================
INIT Init
NEXT Next
INVARIANT Inv
