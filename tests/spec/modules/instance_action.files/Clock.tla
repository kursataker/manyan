---- MODULE Clock ----
EXTENDS Naturals
CONSTANT N
VARIABLE t
Init == t = 0
Up == t' = (t + 1) % N
Next == Up \/ t' = (t + 2) % N
====
