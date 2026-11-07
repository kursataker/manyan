---- MODULE random_subset_draws ----
\* RandomSubset draws indices as TLC does: from a linear congruential
\* generator for an int index, by a 63-bit step past it; a set of
\* records, functions or tuples is indexed by its parts.
\* options: -seed 7
EXTENDS Naturals, TLC, Randomization
ASSUME PrintT(RandomSubset(3, {"c", "a", "b", "d", "e"}))
ASSUME PrintT(RandomSubset(4, 10..30))
ASSUME PrintT(RandomSubset(2, [a : 1..4, b : {"x", "y"}]))
ASSUME PrintT(RandomSubset(3, (1..5) \X {7, 8} \X (0..2)))
ASSUME PrintT(RandomSubset(7, 1..5))
ASSUME PrintT(RandomSubset(0, 1..5))
ASSUME PrintT(RandomSubset(2, {}))
ASSUME PrintT(RandomSubset(2, [1..12 -> 1..8]))
ASSUME PrintT(RandomSubset(2, (1..2000) \X (1..2000) \X (1..2000)))
ASSUME PrintT(RandomSubset(3, SUBSET (1..4)))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
