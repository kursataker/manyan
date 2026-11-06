---- MODULE instance_params ----
\* Instances with substitutions: a parameterized one, two of the same
\* module, a LOCAL one, and a recursive operator through an instance.
EXTENDS Naturals, TLC
VARIABLES a, b
A == INSTANCE Counter WITH Max <- 2, c <- a
B(m) == INSTANCE Counter WITH Max <- m, c <- b
LOCAL C == INSTANCE Counter WITH Max <- 5, c <- 4
Init == A!Init /\ B(3)!Init
Next == A!Next /\ B(3)!Next
Inv == A!Total <= 3 /\ B(3)!Total <= 6 /\ C!Total = 10
====
INIT Init
NEXT Next
INVARIANT Inv
