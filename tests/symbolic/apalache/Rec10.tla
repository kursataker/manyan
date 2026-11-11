\* options: -length 10
\* apalache-mc check Rec10.tla: ERROR (255)
--------------------------- MODULE Rec10 -------------------------------------
(*
 * Test for missing annotation UNROLL_DEFAULT_Fact.
 *
 * Igor Konnov, April 2020
 *)
EXTENDS Integers

VARIABLES
    \* @type: Int;
    f

RECURSIVE Fact(_)

Fact(n) ==
  IF n <= 1
  THEN 1
  ELSE n * Fact(n - 1)

Init ==
    f = Fact(4)

Next ==
    f' = Fact(7)

Inv ==
    f >= 1

=============================================================================
INIT Init
NEXT Next
