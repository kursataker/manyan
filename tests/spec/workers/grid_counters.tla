---- MODULE grid_counters ----
\* TLCGet's counters in a level the workers expand: the search's, as TLC's.
EXTENDS Naturals, TLC
VARIABLES x, y, z
Init == x = 0 /\ y = 0 /\ z = 0
Up(v) == v < 20 /\ v' = v + 1
S == <<TLCGet("distinct"), TLCGet("queue"), TLCGet("diameter")>>
Note == IF x + y + z = 30 /\ x = 19 THEN PrintT(<<x, y, z, S>>) ELSE TRUE
Cap == x + y + z <= 32
Next == /\ Note
        /\ \/ Up(x) /\ UNCHANGED <<y, z>>
           \/ Up(y) /\ UNCHANGED <<x, z>>
           \/ Up(z) /\ UNCHANGED <<x, y>>
====
INIT Init
NEXT Next
CONSTRAINT Cap
