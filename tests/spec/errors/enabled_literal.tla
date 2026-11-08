---- MODULE enabled_literal ----
\* ENABLED of an action with a number as a conjunct: TLC does not evaluate it as a formula.
VARIABLE x
Init == x = 0
Next == ENABLED (x' = 1 /\ 3)
====
INIT Init
NEXT Next
