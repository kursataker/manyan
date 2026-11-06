---- MODULE exit_initial ----
\* TLCSet("exit") in Init: no state is expanded, and the temporal
\* property is checked on no behavior.
EXTENDS Naturals, TLC
VARIABLES n
Init == n \in {0, 1} /\ TLCSet("exit", TRUE)
Next == n < 10 /\ n' = n + 1
Spec == Init /\ [][Next]_n
Prop == <>(n = 10)
====
SPECIFICATION Spec
PROPERTY Prop
