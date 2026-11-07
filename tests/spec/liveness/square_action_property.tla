---- MODULE square_action_property ----
\* A property that is an action [A]_v, not under [], is not a property.
EXTENDS Naturals
VARIABLE x
Next == x < 5 /\ x' = x + 1
Spec == x = 0 /\ [][Next]_x
Prop == [x' = x + 1]_x
====
SPECIFICATION Spec
PROPERTY Prop
