---- MODULE Inner ----
EXTENDS Sequences, Naturals
VARIABLES a, c
Same == ENABLED (c' = c /\ c' = <<1, 2, 3>>)
Filtered == ENABLED (a' = 1 /\ c' = SelectSeq(c, LAMBDA e : e /= a') /\ c' = <<1, 2, 3>>)
====
