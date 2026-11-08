---- MODULE next_if_non_boolean ----
\* An IF's condition in the next-state relation is not a boolean.
VARIABLE x
Init == x = 0
Next == IF 1 THEN x' = 1 ELSE x' = 2
====
INIT Init
NEXT Next
