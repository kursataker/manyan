---- MODULE bag_cardinality_overflow ----
\* BagCardinality adds the counts as Java ints.
EXTENDS Naturals, Bags, TLC
ASSUME BagCardinality(1 :> 2147483647 @@ 2 :> 1) > 0
====
