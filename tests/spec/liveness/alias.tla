---- MODULE alias ----
\* A counterexample's states print as their alias; the last state of the
\* cycle steps to the state it goes back to.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Prop == <>(x = 5)
Alias == [x |-> x, next |-> x']
====
SPECIFICATION Spec
PROPERTY Prop
ALIAS Alias
