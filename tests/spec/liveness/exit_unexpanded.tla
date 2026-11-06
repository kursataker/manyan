---- MODULE exit_unexpanded ----
\* The search stops after expanding n = 3: the state n = 4 it found is
\* not in the graph the property is checked on.
EXTENDS Naturals, TLC
VARIABLES n
Init == n \in {0, 1}
Next == n < 10 /\ n' = n + 1 /\ (n = 3 => TLCSet("exit", TRUE))
Spec == Init /\ [][Next]_n
Prop == <>(n = 10)
====
SPECIFICATION Spec
PROPERTY Prop
