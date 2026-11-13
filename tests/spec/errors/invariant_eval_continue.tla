---- MODULE invariant_eval_continue ----
\* options: -continue
\* With -continue an invariant that fails to evaluate is reported again as
\* an error of the step.
EXTENDS Naturals, Sequences
VARIABLE x
Init == x = 0
Next == x < 2 /\ x' = x + 1
Inv == x < 1 \/ Head(<<>>) = 1
====
INIT Init
NEXT Next
INVARIANT Inv
