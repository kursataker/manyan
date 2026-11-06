---- MODULE constraint_true_warning ----
\* A constraint that is TRUE is dropped as the configuration is read, so
\* liveness checking prints no warning about constraints.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == UNCHANGED x
L == <>(x = 1)
C == TRUE
====
INIT Init
NEXT Next
PROPERTY L
CONSTRAINT C
