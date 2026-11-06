---- MODULE next_constant ----
\* A constant definition named as the next-state action is no action.
VARIABLE x
Init == x = 0
Next == TRUE
====
INIT Init
NEXT Next
