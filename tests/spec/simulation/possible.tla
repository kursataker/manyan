---- MODULE possible ----
\* A simulation counts the _POSSIBLE predicates over all its behaviors.
\* options: -simulate num=3 -depth 4 -seed 1
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = x + 1 \/ x' = x + 2
Three == x = 3
Big == x = 100
====
INIT Init
NEXT Next
_POSSIBLE Three Big
