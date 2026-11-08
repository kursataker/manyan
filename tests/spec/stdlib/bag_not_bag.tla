---- MODULE bag_not_bag ----
\* An argument of (+) that is no bag is Bags' error.
EXTENDS Naturals, Bags, TLC
ASSUME (1 :> 1) (+) (1 :> "a") = EmptyBag
====
