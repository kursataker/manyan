\* options: -length 10 -max-error 10
\* apalache-mc check --inv=Inv --max-error=10 View2.tla: ERROR (255)
---- MODULE View2_2 ----
EXTENDS Integers
VARIABLES
    \* @type: Int;
    x

Init ==
  x = 0

A ==
  x' = x + 1

B ==
  x' = x - 1

C ==
  x' = x

Next ==
  A \/ B \/ C

Inv ==
  x = 0

\* @type: <<Bool, Bool>>;
View1 ==
  <<x < 0, x > 0>>
=================
INIT Init
NEXT Next
INVARIANT Inv
