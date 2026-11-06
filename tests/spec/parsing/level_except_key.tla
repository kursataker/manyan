---- MODULE level_except_key ----
\* An EXCEPT key may not be temporal; SANY reports it at the update.
VARIABLE x
Init == x = [a |-> 1]
Next == x' = [x EXCEPT ![[](x = x)] = 2]
====
INIT Init
NEXT Next
