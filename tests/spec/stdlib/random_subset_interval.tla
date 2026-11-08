---- MODULE random_subset_interval ----
\* RandomSubset of an interval indexes it without making its elements,
\* up to 2^31 - 1 of them; a negative count is Java's
\* NegativeArraySizeException, whatever the set.
EXTENDS Integers, FiniteSets, TLC, Randomization
ASSUME Cardinality(RandomSubset(2, 1..2147483647)) = 2
ASSUME RandomSubset(1, 5..5) = {5}
ASSUME RandomSubset(1, -3..-1) \subseteq -3..-1
ASSUME RandomSubset(-1, {})
====
