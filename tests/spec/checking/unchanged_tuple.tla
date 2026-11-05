---- MODULE unchanged_tuple ----
EXTENDS Naturals
VARIABLES a, b, c
vars == <<a, b, c>>
Init == a = 1 /\ b = 2 /\ c = 3
Next == \/ a < 3 /\ a' = a + 1 /\ UNCHANGED <<b, c>>
        \/ UNCHANGED vars
====
INIT Init
NEXT Next
