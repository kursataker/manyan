---- MODULE stutter_safety ----
\* A property whose negation is a tableau without cycle conditions, as
\* that of <>P: TLC does not warn of its stuttering counterexample.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x
Prop == <>(x = 9)
====
SPECIFICATION Spec
PROPERTY Prop
CHECK_DEADLOCK FALSE
