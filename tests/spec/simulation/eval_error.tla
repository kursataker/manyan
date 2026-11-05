---- MODULE eval_error ----
\* An evaluation error in an action, in a random behavior.
\* options: -simulate -depth 10 -seed 4
EXTENDS Integers
VARIABLE x
Init == x = 3
Next == x' = 6 \div (x - 1) \/ x' = x - 1
====
INIT Init
NEXT Next
