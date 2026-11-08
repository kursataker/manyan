---- MODULE property_instance_operator ----
\* Through an instance, an operator argument reaches a property whose
\* level depends on it: TLC levels every argument of a parameter applied.
EXTENDS Naturals
VARIABLE s
abs == INSTANCE property_instance_operator_A
Init == s = 0
Next == s' = (s + 1) % 3
Spec == Init /\ [][Next]_s
True(e) == TRUE
Small(e) == e < 5
Property == abs!Double(True, s) /\ abs!Double(Small, s) /\ abs!Property(Small, s)
====
SPECIFICATION Spec
PROPERTY Property
