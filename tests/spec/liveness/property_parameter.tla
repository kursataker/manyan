---- MODULE property_parameter ----
\* A property calling a definition with arguments is split through it, as
\* TLC does: []P of a state predicate is an invariant named by the
\* definition, its parameter as state-level as its argument.
EXTENDS Naturals
VARIABLE s
Init == s = 0
Next == s' = (s + 1) % 3
Spec == Init /\ [][Next]_s
Prop(a) == [](a < 2)
Property == Prop(s)
====
SPECIFICATION Spec
PROPERTY Property
