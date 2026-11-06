---- MODULE substitute_cycle ----
\* Init is substituted by a definition that refers to Init: the
\* specification's conjuncts refer to each other forever.
VARIABLE x, z
Init == x = FALSE
Next == x' = ~x /\ z' = z
Spec == Init /\ [][Next]_<<x, z>>
ReDefInit == x \in BOOLEAN /\ z = TRUE /\ Init
====
SPECIFICATION Spec
CONSTANT Init <- ReDefInit
