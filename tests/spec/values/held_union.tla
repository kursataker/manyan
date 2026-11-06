---- MODULE held_union ----
\* A union of sets TLC holds as their definitions, such as sets of records,
\* is kept as its definition: Big, never enumerated, has 10^8 elements.
EXTENDS Naturals, FiniteSets, TLC
R == [a : {1}] \cup [b : {1}] \cup [c : {1}]
Big == [a : 1..100, b : 1..100, c : 1..100, d : 1..100] \cup [e : {1}]
ASSUME PrintT(<<R, Cardinality(R), [a : {}] \cup [b : {}] \cup [c : {}], {[a |-> 2]} \cup [b : {1}] \cup [c : {1}]>>)
ASSUME [e |-> 1] \in Big
====
