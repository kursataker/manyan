---- MODULE Pair ----
VARIABLES u, v
Both == ENABLED (u' = 0 /\ v' = 1)
Apart(A) == ENABLED A
Step == u' = 0 /\ v' = 1
====
