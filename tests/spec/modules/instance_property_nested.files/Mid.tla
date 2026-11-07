---- MODULE Mid ----
EXTENDS Naturals
VARIABLE y, z
Init == y = 0 /\ z = 0
Next == y' = (y + 1) % 3 /\ z' = 1 - z
Spec == Init /\ [][Next]_<<y, z>> /\ WF_<<y, z>>(Next)
M == INSTANCE N WITH x <- y
Refinement == M!Spec
====
