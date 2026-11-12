---- MODULE deadlock_fair ----
\* One behavior stops, another goes on: with fairness, the verdict is
\* TLC's, a deadlock where any behavior stops.
EXTENDS Integers
VARIABLE
    \* @type: Int;
    x
Init == x \in {0, 10}
Next == (x < 2 /\ x' = x + 1) \/ (x >= 10 /\ x' = x)
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Live == <>(x = 2)
====
SPECIFICATION Spec
PROPERTY Live
