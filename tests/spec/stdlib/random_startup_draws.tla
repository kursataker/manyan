---- MODULE random_startup_draws ----
\* Init goes on drawing from where the assumptions stopped, as TLC's main
\* thread keeps the generator its first draw made; the trace rebuilds the
\* initial state with the same draws.
\* options: -seed 7
EXTENDS Integers, TLC, Randomization
ASSUME PrintT(RandomSubset(2, 1..10))
VARIABLE x, y
Init == x \in RandomSubset(3, 1..100000000) /\ y = 0
Next == x' = x /\ y' = y + 1
Inv == y < 1
====
INIT Init
NEXT Next
INVARIANT Inv
