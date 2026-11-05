---- MODULE prefix_conflict ----
\* DOMAIN binds at 10-13 in SANY, so it conflicts with + (10).
EXTENDS Naturals
ASSUME DOMAIN <<1>> + 1 = 2
====
