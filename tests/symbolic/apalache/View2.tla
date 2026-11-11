\* options: -length 10 -max-error 50
\* apalache-mc check --inv=Inv --max-error=50 --view=View1 View2.tla: ERROR (12)
---- MODULE View2 ----
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
VIEW View1
