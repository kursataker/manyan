---- MODULE lazy_context_domain ----
\* A lazy function applied in an action outside its domain: TLC binds the
\* argument itself and reports it as a RuntimeException at the function.
EXTENDS Naturals
VARIABLE x
F[n \in 0..2] == x' = n
Init == x = 0
Next == F[x + 3]
====
INIT Init
NEXT Next
