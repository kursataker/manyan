---- MODULE substitute_arity ----
\* An operator substituted by one of another number of arguments.
EXTENDS Naturals, TLC
F(a, b) == a + b
VARIABLE x
G(a) == a
Init == x = 0
Next == x' = (x + 1) % 3
====
CONSTANT F <- G
INIT Init
NEXT Next
