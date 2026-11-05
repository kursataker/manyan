---- MODULE next_assert ----
\* A failed Assert in Next.
\* options: -simulate -depth 10 -seed 1
EXTENDS Integers, Sequences, TLC
VARIABLE x
Init == x = 0
Next == Assert(x < 3, "too big") /\ x' = x + 1
====
INIT Init
NEXT Next
