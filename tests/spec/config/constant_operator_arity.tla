---- MODULE constant_operator_arity ----
\* The lines of a constant operator must agree on its arity.
CONSTANT F(_, _)
VARIABLE x
Init == x = F(0, "a")
Next == x' = x
====
INIT Init
NEXT Next
CONSTANT
  F(0, "a") = 1
  F(1) = 2
