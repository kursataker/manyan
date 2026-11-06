---- MODULE constraint_warning ----
\* A state constraint while checking a temporal property draws TLC's
\* warning, before the initial states.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Live == <>(x = 2)
Small == x < 5
====
SPECIFICATION Spec
CONSTRAINT Small
PROPERTY Live
