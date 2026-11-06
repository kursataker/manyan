---- MODULE instance_constant ----
\* A definition of an instance that is a constant is computed as one, as
\* the module's own are: in an action it makes no choices (B9 from
\* tlaplus-x/tlx: 3 states generated, not 7).
EXTENDS Naturals
VARIABLE x
I == INSTANCE Branches WITH N <- 3
Init == x = 0
Next == I!Branch /\ x' = (x + 1) % 2
====
INIT Init
NEXT Next
