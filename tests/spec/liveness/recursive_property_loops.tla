---- MODULE recursive_property_loops ----
\* A RECURSIVE property that never ends overflows TLC's stack.
VARIABLE x
RECURSIVE bad(_)
bad(u) == IF u THEN [][TRUE]_x ELSE bad(u)
Spec == x = TRUE /\ [][x' = ~x]_x
Prop == bad(FALSE)
====
SPECIFICATION Spec
PROPERTY Prop
