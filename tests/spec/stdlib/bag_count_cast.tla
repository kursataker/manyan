---- MODULE bag_count_cast ----
\* (-) casts each count to an integer, which fails on a string.
EXTENDS Naturals, Bags, TLC
ASSUME (1 :> "a") (-) (1 :> 1) = EmptyBag
====
