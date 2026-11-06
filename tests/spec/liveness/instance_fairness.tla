---- MODULE instance_fairness ----
\* The fairness of an instance's specification, as a property: in ENABLED,
\* an instance's variable is bound by name, and <<A>>_v compares v so, not
\* through a substitution by an expression of unassigned variables.
EXTENDS Naturals
VARIABLES a, b
pair == <<a, b>>
Init == a = 0 /\ b = 0
Next == a < 2 /\ a' = a + 1 /\ UNCHANGED b
Spec == Init /\ [][Next]_<<a, b>> /\ WF_<<a, b>>(Next)
A == INSTANCE Abs WITH d <- pair[2] + 0, e <- a
Refines == A!Spec
====
SPECIFICATION Spec
PROPERTY Refines
CHECK_DEADLOCK FALSE
