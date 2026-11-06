---- MODULE substitute_chain ----
\* A substitution whose right-hand side is itself substituted for.
EXTENDS Naturals, TLC
CONSTANT A
VARIABLE x
G(a) == a
Init == x = 0
Next == x' = (x + 1) % 3
====
CONSTANT A <- B
         B <- G
INIT Init
NEXT Next
