---- MODULE property_fairness_cycle ----
\* A property WF_v(A) never met under the specification's fairness: the
\* counterexample is a fair cycle, not stuttering, which WF_v(Next) rules out.
EXTENDS Naturals
VARIABLE y, z
Init == y = 0 /\ z = 0
Next == y' = (y + 1) % 3 /\ z' = 1 - z
Spec == Init /\ [][Next]_<<y, z>> /\ WF_<<y, z>>(Next)
A == y' = (y + 1) % 3 /\ UNCHANGED z
Prop == WF_<<y, z>>(A)
====
SPECIFICATION Spec
PROPERTY Prop
