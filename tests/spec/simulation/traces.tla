---- MODULE traces ----
\* A number of random behaviors without an error.
\* options: -simulate num=3 -depth 5 -seed 7
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 5
====
INIT Init
NEXT Next
