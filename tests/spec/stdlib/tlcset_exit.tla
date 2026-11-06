---- MODULE tlcset_exit ----
\* TLCSet("exit", TRUE) stops the search once the state whose expansion
\* asks for it is expanded; the states found but not expanded are left on
\* the queue, and the properties are checked on the graph of the expanded
\* ones (LX2 and LX4 from tlaplus-x/tlx).
EXTENDS Naturals, TLC
VARIABLES n, m
Init == n = 0 /\ m \in 1..300
Next == /\ n < 5
        /\ n' = n + 1
        /\ UNCHANGED m
        /\ IF n = 2 /\ m = 150 THEN TLCSet("exit", TRUE) ELSE TRUE
Spec == Init /\ [][Next]_<<n, m>> /\ WF_<<n, m>>(Next)
Prop == <>[](n < 4)
====
SPECIFICATION Spec
PROPERTY Prop
