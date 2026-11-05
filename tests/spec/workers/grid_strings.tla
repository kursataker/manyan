---- MODULE grid_strings ----
\* Successors hold strings no state held before.
EXTENDS Naturals, Sequences
VARIABLES x, y, z, s
Init == x = 0 /\ y = 0 /\ z = 0 /\ s = "a"
Up(v) == v < 20 /\ v' = v + 1
Next == \/ Up(x) /\ UNCHANGED <<y, z, s>>
        \/ Up(y) /\ UNCHANGED <<x, z, s>>
        \/ Up(z) /\ s' = s \o "b" /\ UNCHANGED <<x, y>>
Short == Len(s) < 12 \/ x + y < 19
====
INIT Init
NEXT Next
INVARIANT Short
