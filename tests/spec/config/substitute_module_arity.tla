---- MODULE substitute_module_arity ----
\* A module's operator substituted by one of another number of
\* arguments.
EXTENDS Naturals, TLC
VARIABLE x
G(a) == a
Init == x = 0
Next == x' = (x + 1) % 3
====
CONSTANT Print <- [TLC] G
INIT Init
NEXT Next
