---- MODULE dnf_weak_fairness ----
\* A WF is one disjunct of TLC's disjunctive normal form, []<>(~ENABLED
\* <<A>>_v \/ <<A>>_v): 31 of them are within a Java int.
EXTENDS Integers
VARIABLE x
Init == x = FALSE
Next == x' \in BOOLEAN
Spec == Init /\ [][Next]_x /\ \A i \in 1..31 : WF_x(Next)
Prop == <>[](x \in BOOLEAN)
====
SPECIFICATION Spec
PROPERTY Prop
