---- MODULE reals_state ----
\* A decimal number in an action.
EXTENDS Reals
VARIABLE x
Init == x = 0
Next == x' = 0.25
====
INIT Init
NEXT Next
