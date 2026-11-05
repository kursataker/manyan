---- MODULE record_field ----
EXTENDS TLC
ASSUME PrintT([[a |-> 1] EXCEPT !.a = 2].b)
====
