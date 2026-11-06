---- MODULE random_set_of_subsets ----
\* RandomSetOfSubsets draws k subsets of a finite set, without enumerating
\* SUBSET SUBSET S; its arguments are checked.
EXTENDS Naturals, FiniteSets, TLC, Randomization
ASSUME PrintT(RandomSetOfSubsets(5, 3, {1, 2, 3}))
ASSUME PrintT(RandomSetOfSubsets(5, 0, {1, 2, 3}))
ASSUME PrintT(\A T \in RandomSetOfSubsets(3, 2, 1..20) : T \subseteq 1..20)
ASSUME PrintT(RandomSetOfSubsets(2, 4, {1, 2, 3}))
====
