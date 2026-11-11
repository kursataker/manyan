---- MODULE tuples ----
\* Pairs from a product, and a tuple-bound quantifier.
EXTENDS Integers
VARIABLES
    \* @type: <<Int, Int>>;
    p
Init == p = <<0, 0>>
Next == \E <<a, b>> \in (0..3) \X (0..3) : a + b = p[1] + 1 /\ p' = <<a, b>>
Inv == p[1] + p[2] < 3
====
INIT Init
NEXT Next
INVARIANT Inv
