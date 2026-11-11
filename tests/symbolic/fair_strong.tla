\* options: -length 6
---- MODULE fair_strong ----
\* Strong fairness of Go, enabled infinitely often, takes it.
VARIABLES
    \* @type: Bool;
    b,
    \* @type: Int;
    x
vars == <<b, x>>
Init == b = FALSE /\ x = 0
Toggle == b' = ~b /\ UNCHANGED x
Go == b /\ x = 0 /\ x' = 1 /\ UNCHANGED b
Next == Toggle \/ Go
Spec == Init /\ [][Next]_vars /\ WF_vars(Next) /\ SF_vars(Go)
Done == <>(x = 1)
====
SPECIFICATION Spec
PROPERTY Done
