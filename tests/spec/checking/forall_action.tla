---- MODULE forall_action ----
\* In an action, \A is the conjunction of its body for each binding, so a
\* disjunction in it is a choice: TLC generates a state for each way.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == /\ x < 3
        /\ \A n \in {1, 2} : n > 0 \/ n < 5
        /\ \A n, m \in {1, 2} : n > 0 \/ m > 0
        /\ \A n \in {} : FALSE
        /\ x' = x + 1
====
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
