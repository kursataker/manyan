---- MODULE constant_operator_undefined ----
\* A constant operator applied to arguments it has no line for.
EXTENDS Naturals
CONSTANT F(_, _)
VARIABLE x
Init == x = 0
Next == x' = F(x, "a")
====
INIT Init
NEXT Next
CONSTANT
  F(0, "a") = 1
  F(1, "a") = 5
