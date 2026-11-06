---- MODULE stack_overflow_initial ----
\* A recursive function that never ends, applied in Init: TLC's Java
\* stack overflows.
EXTENDS Naturals
VARIABLE x
f[n \in Nat] == f[n + 1]
Init == x = f[0]
Next == UNCHANGED x
====
INIT Init
NEXT Next
