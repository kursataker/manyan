---- MODULE print_infinite_union ----
\* TLC enumerates a set to print it: an infinite union fails.
EXTENDS Integers, TLC
ASSUME Print("u", Nat \cup {-1})
====
