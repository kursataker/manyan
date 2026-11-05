---- MODULE grid ----
\* Levels wider than workers take in one batch: the search spreads them.
EXTENDS Naturals
VARIABLES x, y, z
vars == <<x, y, z>>
Init == x = 0 /\ y = 0 /\ z = 0
Up(v) == v < 20 /\ v' = v + 1
Next == \/ Up(x) /\ UNCHANGED <<y, z>>
        \/ Up(y) /\ UNCHANGED <<x, z>>
        \/ Up(z) /\ UNCHANGED <<x, y>>
        \/ x + y + z = 60 /\ UNCHANGED vars
Spec == Init /\ [][Next]_vars /\ WF_vars(Next)
Bounded == x + y + z <= 60
Corner == <>(x + y + z = 60)
====
SPECIFICATION Spec
INVARIANT Bounded
PROPERTY Corner
