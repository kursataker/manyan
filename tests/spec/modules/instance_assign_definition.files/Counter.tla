---- MODULE Counter ----
EXTENDS Naturals
CONSTANT V
VARIABLE m
Start == m = V + 2
Step == m' = m + 1 /\ m < V + 4
====
