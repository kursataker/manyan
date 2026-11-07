---- MODULE constant_error ----
\* A constant in a temporal property that fails: TLC evaluates it as it
\* builds its tableau, before the initial states, and reports an
\* unexpected exception. Found by tlaplus/Examples' TestGraphs.
EXTENDS Naturals, TLC
VARIABLE x
C == CHOOSE r \in [STRING -> STRING] : TRUE
ASSUME PrintT("assumed")
Init == x = 0
Next == x' = 1 - x
Live == C # {} => <>[](x = 1)
====
INIT Init
NEXT Next
PROPERTY Live
