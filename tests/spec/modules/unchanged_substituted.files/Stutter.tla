---- MODULE Stutter ----
EXTENDS Naturals
VARIABLES s, vars
Pause == s' = 1 - s /\ UNCHANGED vars
====
