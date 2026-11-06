---- MODULE tuple_source_except ----
\* A tuple EXCEPT makes is a new tuple, with no expression TLC names.
EXTENDS Naturals
VARIABLE t
Init == t = [<<1, 2>> EXCEPT ![1] = 3]
Inv == t[3] = 0
Next == UNCHANGED t
====
INIT Init
NEXT Next
INVARIANT Inv
