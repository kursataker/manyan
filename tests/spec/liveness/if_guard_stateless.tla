---- MODULE if_guard_stateless ----
\* TLC evaluates an IF guard of a property with no state: one that is
\* decided before it reads a variable chooses its branch.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = 1 - x
Spec == Init /\ [][Next]_x
Prop == IF TRUE \/ x = 5 THEN <>(x = 1) ELSE [](x = 0)
====
SPECIFICATION Spec
PROPERTY Prop
