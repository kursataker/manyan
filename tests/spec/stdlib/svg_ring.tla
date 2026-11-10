---- MODULE svg_ring ----
\* SVG's NodeOfRingNetwork(cx, cy, r, n, m), which TLC implements in Java:
\* node n of m on a circle of radius r about (cx, cy), each coordinate a
\* double cut to an int. Found by tlaplus/Examples' EWD840_anim.
EXTENDS SVG, TLC, Naturals
ASSUME PrintT([n \in 0..5 |-> NodeOfRingNetwork(100, 50, 40, n, 6)])
ASSUME PrintT(NodeOfRingNetwork(0, 0, 7, 3, 0))
====
