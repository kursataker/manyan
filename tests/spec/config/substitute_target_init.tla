---- MODULE substitute_target_init ----
\* The initial predicate is the right-hand side of a substitution, which
\* refers to the operator it substitutes for.
VARIABLE x, z
Init == x = FALSE
Next == x' = ~x /\ z' = z
ReDefInit == x \in BOOLEAN /\ z = TRUE /\ Init
====
INIT ReDefInit
NEXT Next
CONSTANT Init <- ReDefInit
