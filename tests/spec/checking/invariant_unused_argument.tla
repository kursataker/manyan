---- MODULE invariant_unused_argument ----
\* An invariant that passes a variable to a parameter its definition does
\* not use is constant by SANY's levels, yet TLC counts the argument and
\* checks it in each state.
EXTENDS Naturals
VARIABLE t
Init == t = 1
Next == t' = t
Never(arr) == FALSE
Inv == Never(t)
====
INIT Init
NEXT Next
INVARIANT Inv
