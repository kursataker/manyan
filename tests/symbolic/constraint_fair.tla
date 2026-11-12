\* options: -length 6
---- MODULE constraint_fair ----
\* The action constraint cuts the wrap from 3 to 0, and x = 7 is outside
\* the constraint: every fair behavior TLC explores reaches 3.
EXTENDS Naturals
VARIABLE
    \* @type: Int;
    x
Init == x \in {0, 7}
Next == x' = (x + 1) % 4
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Small == x < 5
NoWrap == ~(x = 3 /\ x' = 0)
Three == <>(x = 3)
====
SPECIFICATION Spec
CONSTRAINT Small
ACTION_CONSTRAINT NoWrap
PROPERTY Three
