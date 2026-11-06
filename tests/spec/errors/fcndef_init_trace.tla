---- MODULE fcndef_init_trace ----
\* The initial predicate computes a function definition in full, as TLC
\* does: an error doing so is located at the whole definition.
EXTENDS Naturals
VARIABLE x
F[n \in 0..2] == x = n
Init == F[1]
Next == UNCHANGED x
====
INIT Init
NEXT Next
