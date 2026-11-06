---- MODULE instance_action ----
\* An action of an instance that substitutes is not split: it is one
\* action, I!A, located at the INSTANCE definition.
EXTENDS Naturals
VARIABLE t
C == INSTANCE Clock WITH N <- 3
Init == C!Init
Next == C!Next
Small == t < 1
====
INIT Init
NEXT Next
INVARIANT Small
