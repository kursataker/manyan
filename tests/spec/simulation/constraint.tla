---- MODULE constraint ----
\* A successor outside the constraint is never taken: x = 3 deadlocks.
\* options: -simulate num=4 -depth 10 -seed 5
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = x + 1 \/ x' = x + 2
Small == x < 4
Inv == x < 7
====
INIT Init
NEXT Next
CONSTRAINT Small
INVARIANT Inv
