---- MODULE random_subset ----
\* RandomSubset draws k elements of a finite set, without enumerating
\* SUBSET S (SmokeEWD998 from lemmy/ewd998); its arguments are checked.
EXTENDS Naturals, FiniteSets, TLC, Randomization
ASSUME PrintT(RandomSubset(5, {1, 2, 3}))
ASSUME PrintT(Cardinality(RandomSubset(3, [1..4 -> 1..9])))
ASSUME PrintT(RandomSubset(1, Nat))
====
