---- MODULE dnf_too_large ----
\* TLC's disjunctive normal form of the fairness and the negated property
\* has 2^31 disjuncts, each SF two: more than a Java int counts. A WF is
\* one.
EXTENDS Integers
VARIABLE x
Init == x = FALSE
Next == x' \in BOOLEAN
Spec == Init /\ [][Next]_x /\ \A i \in 1..31 : SF_x(Next) /\ WF_x(Next)
Prop == <>[](x \in BOOLEAN)
====
SPECIFICATION Spec
PROPERTY Prop
