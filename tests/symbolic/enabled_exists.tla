---- MODULE enabled_exists ----
\* ENABLED of an action that chooses: Pick is disabled at x = 2, where no
\* y it may choose is between x and 3.
EXTENDS Naturals
VARIABLE
    \* @type: Int;
    x
Init == x = 0
Next == x < 5 /\ x' = x + 1
Pick == \E y \in 0..5 : y > x /\ y < 3 /\ x' = y
PickOn == ENABLED Pick
====
INIT Init
NEXT Next
INVARIANT PickOn
