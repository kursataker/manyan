---- MODULE random_seed ----
\* Model checking draws the model's values from -seed, as TLC.
\* options: -seed 42
EXTENDS Naturals, TLC, Randomization
ASSUME PrintT(<<RandomElement(1..1000), RandomSubset(3, [1..3 -> 1..9])>>)
ASSUME PrintT(RandomSetOfSubsets(3, 2, 1..6))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
