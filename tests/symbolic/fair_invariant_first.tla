\* options: -length 8 -deadlock
---- MODULE fair_invariant_first ----
\* The behavior from x = 10 violates Live at once, but TLC checks the
\* invariants of every state before any property.
EXTENDS Integers
VARIABLE
    \* @type: Int;
    x
Init == x \in {0, 10}
Next == (x < 4 /\ x' = x + 1) \/ (x >= 10 /\ x' = 10)
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Small == x < 4 \/ x = 10
Live == <>(x = 4)
====
SPECIFICATION Spec
INVARIANT Small
PROPERTY Live
