---- MODULE spec_constant_true ----
\* A conjunct of a specification that is a constant TRUE is dropped: with
\* no variables there is then no initial state (IsEven from
\* miguelmota/tla-cookbook); with one, no initial predicate.
EXTENDS TLC
Init == TRUE
Next == Print("next", TRUE)
Spec == Init /\ [][Next]_<<>>
====
SPECIFICATION Spec
