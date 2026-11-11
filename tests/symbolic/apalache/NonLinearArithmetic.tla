\* options: -length 0
\* apalache-mc check --smt-solver=cvc5 --length=0 --inv=SquareNonNegative NonLinearArithmetic.tla: ERROR (255)
-------------------------- MODULE NonLinearArithmetic --------------------------
EXTENDS Integers

VARIABLE
  \* @type: Int;
  x

Init == x \in 1..3

Next == x' = x

SquareNonNegative == x * x >= 0

===============================================================================
INIT Init
NEXT Next
INVARIANT SquareNonNegative
