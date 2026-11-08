---- MODULE constant_operator_values ----
\* The configuration defines a constant operator by its values.
CONSTANT F(_)
VARIABLE x
Init == x = 0
Next == x' = F(x)
Inv == x /= 2
====
INIT Init
NEXT Next
CONSTANT
    F(0) = 1
    F(1) = 2
    F(2) = 2
INVARIANT Inv
