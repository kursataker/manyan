---- MODULE enabled_if_non_boolean ----
\* ENABLED of an IF whose condition is not a boolean.
VARIABLE x
Init == x = 0
Next == ENABLED (IF 1 THEN x' = 1 ELSE TRUE)
====
INIT Init
NEXT Next
