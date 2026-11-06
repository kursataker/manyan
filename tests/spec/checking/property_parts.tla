---- MODULE property_parts ----
\* A property is split into its conjuncts, through a call of an instance's
\* definition, as TLC: []P an invariant and [][A]_v an action property,
\* each named, as one of several, by its location.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = x + 2 /\ x < 4
Spec == Init /\ [][Next]_x
A == INSTANCE Ab
Holds == Init /\ [](x < 9)
Refines == A!Spec /\ Init
====
SPECIFICATION Spec
PROPERTY Holds Refines
CHECK_DEADLOCK FALSE
