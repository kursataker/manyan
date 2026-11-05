---- MODULE record_state ----
EXTENDS Naturals
VARIABLES r
Init == r = [a |-> 1]
Next == r' = [r EXCEPT !.a = 2]
Inv == r.a = 1 \/ r.b = 0
====
INIT Init
NEXT Next
INVARIANT Inv
