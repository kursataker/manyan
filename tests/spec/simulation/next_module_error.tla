---- MODULE next_module_error ----
\* An error of a standard module's operator in Next: an EvalException.
\* options: -simulate -depth 10 -seed 1
EXTENDS Integers, Sequences, TLC
VARIABLE x
Init == x = 0
Next == x' = 6 \div (3 - x)
====
INIT Init
NEXT Next
