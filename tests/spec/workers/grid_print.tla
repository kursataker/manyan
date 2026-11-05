---- MODULE grid_print ----
\* Output a level prints comes out in the order the search expands it.
EXTENDS Naturals, TLC
VARIABLES x, y, z
Init == x = 0 /\ y = 0 /\ z = 0
Up(v) == v < 20 /\ v' = v + 1
Note == IF x + y + z = 30 /\ x = 19 THEN PrintT(<<x, y, z>>) ELSE TRUE
Cap == x + y + z <= 32
Next == /\ Note
        /\ \/ Up(x) /\ UNCHANGED <<y, z>>
           \/ Up(y) /\ UNCHANGED <<x, z>>
           \/ Up(z) /\ UNCHANGED <<x, y>>
====
INIT Init
NEXT Next
CONSTRAINT Cap
