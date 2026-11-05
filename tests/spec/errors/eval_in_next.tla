---- MODULE eval_in_next ----
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = 10 \div (2 - x)
====
INIT Init
NEXT Next
