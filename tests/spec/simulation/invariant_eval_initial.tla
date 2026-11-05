---- MODULE invariant_eval_initial ----
\* An invariant that fails to evaluate on an initial state.
\* options: -simulate -depth 10 -seed 1
EXTENDS Integers, Sequences, TLC
VARIABLE x
Init == x \in {0, 3}
Next == x' = x + 1
Inv == 6 \div (3 - x) > 0
====
INIT Init
NEXT Next
INVARIANT Inv
