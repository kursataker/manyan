---- MODULE random_draws ----
\* A simulation draws as TLC: one stream for the model, one for the
\* behaviors, whatever the seed, and functions, records and tuples draw
\* part by part; a violated behavior is rebuilt drawing again.
\* options: -simulate num=2 -depth 4 -seed 7
EXTENDS Naturals, TLC, Randomization
VARIABLE x, y
ASSUME PrintT(<<RandomElement(1..100), RandomElement([a: 1..3, b: {"p", "q"}])>>)
Init == x = RandomElement([1..2 -> 0..9]) /\ y = 0
Next == y < 3
     /\ PrintT(<<y, RandomSubset(2, (1..9) \X (1..9)), RandomSetOfSubsets(2, 2, 1..5)>>)
     /\ x' = RandomElement([1..2 -> 0..9])
     /\ y' = y + 1
Inv == x[1] # 0
====
INIT Init
NEXT Next
INVARIANT Inv
CHECK_DEADLOCK FALSE
