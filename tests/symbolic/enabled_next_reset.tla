\* options: -length 6 -deadlock
---- MODULE enabled_next_reset ----
\* ENABLED in an action: x resets once Up is disabled, at n = 3.
EXTENDS Naturals
VARIABLES
    \* @type: Int;
    x,
    \* @type: Int;
    n
Init == x = 0 /\ n = 0
Up == x < 2 /\ x' = x + 1
Next == n < 6 /\ n' = n + 1 /\ ((ENABLED Up /\ Up) \/ (~ENABLED Up /\ x' = 0))
Inv == ~(x = 0 /\ n \in {1, 2})
Reset == ~(x = 0 /\ n = 3)
====
INIT Init
NEXT Next
INVARIANT Reset
