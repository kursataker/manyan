---- MODULE invariant_eval ----
\* An invariant that fails to evaluate on a successor: TLC's report, the successor last.
\* options: -simulate -depth 10 -seed 1
EXTENDS Integers, Sequences, TLC
VARIABLE x
Init == x = 0
Next == x' = x + 1
Inv == 6 \div (3 - x) > 0
====
INIT Init
NEXT Next
INVARIANT Inv
