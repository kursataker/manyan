---- MODULE unassigned_continue ----
\* options: -continue
\* With -continue TLC reports its failure to look the successor up for the
\* postconditions, and the error again as it replays it.
VARIABLES x, y
Init == x = 1 /\ y = 1
Next == x' = 2
====
INIT Init
NEXT Next
