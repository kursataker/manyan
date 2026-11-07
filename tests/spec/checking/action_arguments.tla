---- MODULE action_arguments ----
\* A step is labeled with the values of its action's constant arguments.
EXTENDS Naturals
VARIABLE c
Init == c = [p \in {1, 2} |-> 0]
Inc(p, q) == c[p] < 2 /\ c' = [c EXCEPT ![p] = @ + q]
Dec(p) == c[p] > 5 /\ c' = c
Next == (\E p \in {1, 2} : Inc(p, p + 1)) \/ (\E r \in {"x"} : Dec(1))
Inv == c[2] < 1
====
INIT Init
NEXT Next
INVARIANT Inv
