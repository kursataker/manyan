---- MODULE missing_init_invariant ----
\* The specification's initial predicate is TRUE, so dropped: with an
\* invariant to check, TLC finds no initial predicate.
EXTENDS Naturals
Init == TRUE
Next == TRUE
Spec == Init /\ [][Next]_<<>>
Inv == 1 + 1 = 2
====
SPECIFICATION Spec
INVARIANT Inv
