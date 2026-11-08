---- MODULE error_initial_status ----
\* An error in a property on an initial state exits with the error's
\* status: an undefined identifier is a configuration error's 151.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == (x = 0 /\ x' = 1) \/ UNCHANGED x
Spec == Init /\ [][Next]_x
Prop == <>~ENABLED (x' > x)
====
SPECIFICATION Spec
PROPERTY Prop
