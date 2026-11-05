---- MODULE next_error ----
\* An error of TLC's evaluator in Next: a RuntimeException, with the behavior.
\* options: -simulate -depth 10 -seed 1
EXTENDS Integers, Sequences, TLC
VARIABLE x
Init == x = 0
Next == x' = CHOOSE y \in {x + 1, x + 2} : y > x + 2 \/ x < 2
====
INIT Init
NEXT Next
