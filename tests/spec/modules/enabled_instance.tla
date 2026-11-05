---- MODULE enabled_instance ----
\* Inside ENABLED, a variable of the instance written in the action itself
\* is bound by name; an action passed from outside keeps its substitutions.
EXTENDS TLC
VARIABLE z
P == INSTANCE Pair WITH u <- z, v <- z
Init == z = 0
Next == UNCHANGED z
ASSUME PrintT(<<P!Both, P!Apart(P!Step), P!Apart(z' = 1)>>)
====
INIT Init
NEXT Next
