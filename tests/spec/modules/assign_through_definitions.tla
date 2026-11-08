---- MODULE assign_through_definitions ----
\* x' is assigned through a definition, an instance's definition, LET,
\* and a prime of an instance's definition, as TLC's getPrimedVar finds it,
\* in the next-state action and in ENABLED.
EXTENDS Naturals
VARIABLE a
I == INSTANCE Inner WITH x <- a
Op == a'
Init == a = 0
Next == \/ a < 1 /\ I!F \in {1}
        \/ a = 1 /\ (LET y == a' IN y) = 2
        \/ a = 2 /\ I!G' = 3
        \/ a = 3 /\ Op = 4
Inv == /\ ENABLED (I!F \in {0, 1})
       /\ ENABLED ((LET y == a' IN y) = 0)
       /\ ENABLED (I!G' = 0)
       /\ ENABLED (Op = 0)
====
INIT Init
NEXT Next
INVARIANT Inv
CHECK_DEADLOCK FALSE
