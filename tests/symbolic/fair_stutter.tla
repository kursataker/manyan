\* options: -length 6 -deadlock
---- MODULE fair_stutter ----
\* Weak fairness of Next leaves x at 2, where Next is disabled: the
\* behavior stutters there forever.
EXTENDS Naturals
VARIABLE
    \* @type: Int;
    x
Init == x = 0
Next == x < 2 /\ x' = x + 1
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Three == <>(x = 3)
====
SPECIFICATION Spec
PROPERTY Three

