---- MODULE init_if_non_boolean ----
\* An IF's condition in the initial predicate is not a boolean.
VARIABLE x
Init == IF 1 THEN x = 1 ELSE x = 2
Next == x' = x
====
INIT Init
NEXT Next
