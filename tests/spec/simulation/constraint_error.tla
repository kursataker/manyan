---- MODULE constraint_error ----
\* An error in the constraint of an initial state: TLC prints the behavior,
\* the state it was in.
\* options: -simulate num=2 -depth 5 -seed 3
EXTENDS Naturals
VARIABLE x
Init == x = 1
Next == x' = x + 1 /\ x < 3
Bounded == <<1>>[x + 1] = 1
====
INIT Init
NEXT Next
CONSTRAINT Bounded
