---- MODULE warning_before_assume ----
\* TLC warns about constraints in liveness checking as its checker starts,
\* before it evaluates the assumptions.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = 1 - x
L == <>(x = 1)
C == x < 5
ASSUME FALSE
====
INIT Init
NEXT Next
PROPERTY L
CONSTRAINT C
