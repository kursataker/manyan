---- MODULE constraint_nowarning ----
\* -nowarning silences TLC's warnings: that of a constraint while checking
\* a temporal property, and that of an EXCEPT outside the domain.
\* options: -nowarning
EXTENDS Naturals
VARIABLE x
Init == x = [i \in 1..2 |-> 0]
Next == x' = [x EXCEPT ![3] = 1]
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Live == <>(x[1] = 0)
Small == x[1] < 5
====
SPECIFICATION Spec
CONSTRAINT Small
PROPERTY Live
