---- MODULE counter_example_tableau_repeat ----
\* The lasso moves from one tableau node of x = 1 to another, by its self
\* loop and by stuttering: TLC prints the state once.
VARIABLE x
Init == x = 1
Next == x' = 1
Spec == Init /\ [][Next]_x
P == (x = 1) ~> (x = 2)
====
SPECIFICATION Spec
PROPERTY P
