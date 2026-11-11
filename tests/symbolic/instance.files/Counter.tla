---- MODULE Counter ----
EXTENDS Integers
CONSTANT Max
VARIABLE n
Init == n = 0
Next == n < Max /\ n' = n + 1
Small == n < Max
====
