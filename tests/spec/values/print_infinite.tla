---- MODULE print_infinite ----
\* TLC enumerates a set to print it: an infinite union or difference fails.
EXTENDS Integers, TLC
ASSUME PrintT(<<Nat \cap {1}, {1} \cap Nat>>)
ASSUME PrintT(<<1, Nat \ {0}>>)
====
