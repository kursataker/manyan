---- MODULE integer_ops ----
EXTENDS Integers, TLC
ASSUME PrintT(<<-3 .. -1, -5 + 2, -(-5), Int \subseteq Int, 3 - 5>>)
====
