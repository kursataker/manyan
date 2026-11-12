\* options: -length 6
---- MODULE constraint_stutter ----
\* Without fairness a behavior may stutter at x = 0, inside the
\* constraint, and never reach 2.
EXTENDS Naturals
VARIABLE
    \* @type: Int;
    x
Init == x = 0
Next == x' = x + 1
Spec == Init /\ [][Next]_x
Small == x < 3
Two == <>(x = 2)
====
SPECIFICATION Spec
CONSTRAINT Small
PROPERTY Two
