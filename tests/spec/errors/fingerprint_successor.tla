---- MODULE fingerprint_successor ----
\* A successor holding a set TLC keeps as its definition fails when TLC
\* fingerprints the state, after Next: an unexpected exception without the
\* behavior, with the record and set that hold it (spectacle's
\* semantic_error1). EXCEPT does not fingerprint the value early.
EXTENDS Naturals
VARIABLE x
Init == x \in {[a |-> 0], [a |-> 1]}
Next == x' = [x EXCEPT !.a = {x.a \cup {1}}]
====
INIT Init
NEXT Next
