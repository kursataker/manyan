\* options: -length 6
---- MODULE constraint_end ----
\* x = 3 is outside the constraint, so TLC does not expand it, and weak
\* fairness forbids stopping at x = 2: no behavior TLC explores fails.
EXTENDS Naturals
VARIABLE
    \* @type: Int;
    x
Init == x = 0
Next == x' = x + 1
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Small == x < 3
Four == <>(x = 4)
====
SPECIFICATION Spec
CONSTRAINT Small
PROPERTY Four
