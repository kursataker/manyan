---- MODULE spec_action_conjunct ----
\* An action as a conjunct of the specification, not under [][...]_v, is
\* one TLC cannot handle: Next's conjuncts are the spec's.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Spec == Init /\ Next
====
SPECIFICATION Spec
