\* options: -length 0
\* apalache-mc check --smt-solver=cvc5 --tuning-options=cvc5.smt.logic=QF_UFNIA --length=0 --inv=SquareNonNegative NonLinearArithmetic.tla: OK
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
