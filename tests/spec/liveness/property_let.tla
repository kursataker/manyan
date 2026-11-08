---- MODULE property_let ----
\* A property is split through LET, its definitions in scope.
EXTENDS Naturals
VARIABLE s
Init == s = 0
Next == s' = (s + 1) % 3
Spec == Init /\ [][Next]_s
Property == LET q == s < 2 IN [](q)
====
SPECIFICATION Spec
PROPERTY Property
