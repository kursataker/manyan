---- MODULE property_forall ----
\* \A k \in S : P over a constant S is a part for each k, as TLC unrolls
\* it: here invariants, named by the body's location.
EXTENDS Naturals
VARIABLE s
Init == s = 0
Next == s' = (s + 1) % 3
Spec == Init /\ [][Next]_s
Property == \A k \in {1, 2} : [](s < k + 1)
====
SPECIFICATION Spec
PROPERTY Property
