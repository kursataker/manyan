---- MODULE Keep ----
VARIABLES x, y
Spec == [][y = 1 /\ y' = 2 => x' = x]_<<x, y>>
====
