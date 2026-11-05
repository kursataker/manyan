---- MODULE except ----
EXTENDS Naturals, TLC
f == [x \in 1..3 |-> x]
r == [a |-> 1, b |-> [c |-> 2, d |-> <<3, 4>>]]
ASSUME PrintT(<<[f EXCEPT ![2] = 20], [f EXCEPT ![1] = @ + 100, ![3] = 0], [f EXCEPT ![1] = 5, ![1] = @ + 1]>>)
ASSUME PrintT(<<[r EXCEPT !.a = 9], [r EXCEPT !.b.c = @ * 10], [r EXCEPT !.b.d[2] = "x"], [r EXCEPT !["b"]["c"] = 0]>>)
ASSUME PrintT([[x \in 1..2 |-> [y \in 1..2 |-> 0]] EXCEPT ![1][2] = 1, ![2] = <<7>>])
====
