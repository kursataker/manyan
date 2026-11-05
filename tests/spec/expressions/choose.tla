---- MODULE choose ----
EXTENDS Naturals, FiniteSets, TLC
ASSUME PrintT(<<CHOOSE x \in 1..10 : x * x > 20, CHOOSE x \in {"b", "a"} : TRUE>>)
ASSUME PrintT(CHOOSE <<a, b>> \in (1..3) \X (1..3) : a + b = 5)
ASSUME PrintT(CHOOSE s \in SUBSET {1, 2, 3} : Cardinality(s) = 2)
====
