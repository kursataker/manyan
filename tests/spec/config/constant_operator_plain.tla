---- MODULE constant_operator_plain ----
\* A later plain value replaces the lines; applied, it is that value.
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
  F = 2
