---- MODULE constant_operator_wrong_arguments ----
\* Lines of another arity than the declaration's.
EXTENDS Naturals
CONSTANT F(_)
VARIABLE x
Init == x = 0
Next == x' = F(x) + 1
====
INIT Init
NEXT Next
CONSTANT
  F(0, 1) = 1
