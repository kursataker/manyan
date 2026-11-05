---- MODULE operator_param_forward ----
EXTENDS Naturals
Double(x) == 2 * x
Twice(F(_), x) == F(F(x))
Forward(F(_), x) == Twice(F, x)
I(F(_)) == INSTANCE Shifted WITH G <- F
ASSUME Forward(Double, 3) = 12
ASSUME I(Double)!Twice(1) = 4
ASSUME I(LAMBDA y : y + 5)!Twice(1) = 11
====
