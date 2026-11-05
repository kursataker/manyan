---- MODULE apply_state ----
\* Functions in a state: a record stays one, a function from [x \in S |-> e]
\* is applied as TLC applies a stored function.
EXTENDS Naturals
VARIABLES f, r
Init == f = [x \in 1..2 |-> x] /\ r = [a |-> 1]
Next == UNCHANGED <<f, r>>
Inv == f[2] = 2 /\ r.a = 1 /\ (f[1] = 1 => f[3] = 0)
====
INIT Init
NEXT Next
INVARIANT Inv
