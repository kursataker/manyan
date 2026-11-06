---- MODULE bare_action_property ----
\* A property's part that is an action, not under [], is not a property:
\* TLC names the part, here by its place, as the property has two.
EXTENDS Naturals
VARIABLE x
Next == x < 5 /\ x' = x + 1
Spec == x = 0 /\ [][Next]_x
Prop == x < 5 /\ x' = x + 1
====
SPECIFICATION Spec
PROPERTY Prop
