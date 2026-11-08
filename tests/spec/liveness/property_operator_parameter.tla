---- MODULE property_operator_parameter ----
\* A property's operator parameter has the level of the operator and the
\* arguments it is applied to.
EXTENDS Naturals
VARIABLE s
Init == s = 0
Next == s' = (s + 1) % 3
Spec == Init /\ [][Next]_s
Test(e) == e < 2
Prop(ns(_), a) == [](ns(a))
Property == Prop(Test, s) /\ Prop(LAMBDA e : e < 5, s)
====
SPECIFICATION Spec
PROPERTY Property
