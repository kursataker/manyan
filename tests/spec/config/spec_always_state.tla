---- MODULE spec_always_state ----
\* TLC cannot handle []S in the specification, S a state predicate; it
\* takes the specification before the properties, so the property's
\* warning is not printed.
EXTENDS Naturals
VARIABLE x
Init == x = 1
Next == x < 5 /\ x' = x + 1
Spec == Init /\ [][Next]_x /\ [](x < 3)
Inv == x < 4
====
PROPERTY Inv
SPECIFICATION Spec
