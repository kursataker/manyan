---- MODULE next_literal ----
\* A number where TLC expects an action, met while computing a step.
VARIABLE x
Init == x = 0
Next == 3 /\ x' = 1
====
INIT Init
NEXT Next
