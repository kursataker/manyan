---- MODULE instance ----
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
C == INSTANCE Counter WITH n <- x, Max <- 3
P(m) == INSTANCE Counter WITH n <- m, Max <- 10
ASSUME PrintT(<<P(4)!Inc, P(10)!AtMax>>)
Inv == C!Inv
====
INIT Init
NEXT Next
INVARIANT Inv
CHECK_DEADLOCK FALSE
