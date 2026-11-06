---- MODULE substitute_no_module ----
\* A substitution in a module the specification does not have.
EXTENDS Naturals, TLC
VARIABLE x
G(a) == a
Init == x = 0
Next == x' = (x + 1) % 3
====
CONSTANT G <- [NoSuchModule] G
INIT Init
NEXT Next
