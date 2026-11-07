---- MODULE postcondition_initial ----
\* After an initial state violates an invariant, that error is reported as
\* one evaluating the invariant on the initial state.
\* options: -simulate num=2 -depth 5 -seed 2
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Inv == x > 0
====
INIT Init
NEXT Next
INVARIANT Inv
POSTCONDITION Nope
