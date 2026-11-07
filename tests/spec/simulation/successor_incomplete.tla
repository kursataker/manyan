---- MODULE successor_incomplete ----
\* A successor that leaves a variable out: TLC prints the behavior to it, then its trace postcondition fails.
\* options: -simulate -depth 100 -seed 99
EXTENDS Integers, Sequences, TLC
VARIABLES a, b
Init == a \in 0..9 /\ b \in {"x", "y"}
Inc == a' \in {a + 1, a + 2, a + 3} /\ UNCHANGED b
Sw == b' = IF b = "x" THEN "y" ELSE "x" /\ a' = a
Next == Inc \/ Sw
Inv == a < 30
====
INIT Init
NEXT Next
INVARIANT Inv
