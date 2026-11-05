---- MODULE pretty ----
EXTENDS Naturals, TLC
ASSUME PrintT([i \in 1..30 |-> i * 1000])
ASSUME PrintT([n \in 1..3 |-> [field1 |-> "a long string value", field2 |-> {1, 2, 3}, field3 |-> <<"x", "y">>]])
ASSUME PrintT({"process one", "process two", "process three", "process four", "process five"})
====
