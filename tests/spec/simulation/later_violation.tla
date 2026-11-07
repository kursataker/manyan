---- MODULE later_violation ----
\* A successor that violates an invariant after another successor was
\* found still ends the behavior.
\* options: -simulate num=3 -depth 10 -seed 1
EXTENDS Integers
VARIABLE x
Init == x = 5
Next == x' \in {x, x + 1} /\ x < 6
Inv == x \in 1..5
====
INIT Init
NEXT Next
INVARIANT Inv
