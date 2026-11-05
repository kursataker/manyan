---- MODULE held_union ----
\* A union of sets TLC holds as their definitions, such as sets of records.
EXTENDS FiniteSets, TLC
R == [a : {1}] \cup [b : {1}] \cup [c : {1}]
ASSUME PrintT(<<R, Cardinality(R), [a : {}] \cup [b : {}] \cup [c : {}], {[a |-> 2]} \cup [b : {1}] \cup [c : {1}]>>)
====
