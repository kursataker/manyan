---- MODULE subscript_cycle ----
\* A substitution that makes the subscript of the next-state relation
\* refer to itself overflows TLC's stack as it collects the subscript's
\* variables.
EXTENDS Naturals
VARIABLE x, z
vars == <<x>>
ReDefVars == <<vars, z>>
Init == x = 0 /\ z = 0
Next == x' = 1 - x /\ z' = z
Spec == Init /\ [][Next]_vars
====
SPECIFICATION Spec
CONSTANT vars <- ReDefVars
