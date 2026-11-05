---- MODULE merge_argument ----
\* @@ needs two functions.
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT(1 :> 2 @@ 3)
====
