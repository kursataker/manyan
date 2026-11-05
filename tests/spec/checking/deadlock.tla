---- MODULE deadlock ----
EXTENDS Naturals
VARIABLES a, b
Init == a = 0 /\ b = "idle"
Step == a < 2 /\ a' = a + 1 /\ b' = "busy"
Next == Step
====
INIT Init
NEXT Next
