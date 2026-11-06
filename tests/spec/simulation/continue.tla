---- MODULE continue ----
\* -continue in a simulation: a violation ends its behavior, and the
\* next begins; TLC then exits 255.
\* options: -continue -simulate num=2 -depth 6 -seed 1
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 4 /\ x' = x + 1
Inv == x # 2
====
INIT Init
NEXT Next
INVARIANT Inv
CHECK_DEADLOCK FALSE
