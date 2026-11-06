---- MODULE view_trace_rebuilt ----
\* Under a view TLC rebuilds a trace from fingerprints: the first initial
\* state of the view's value, though outside the constraint, then the first
\* successor of each; the violating state is the one found (tlx's
\* ViewConcretePath).
EXTENDS Naturals
VARIABLES x, hidden
vars == <<x, hidden>>
Init == x = 0 /\ hidden \in {0, 1}
Next == \/ UNCHANGED vars
        \/ x' = x + 1 /\ hidden' = hidden + 1
InModel == hidden > 0
Inv == x < 3
View == x
====
INIT Init
NEXT Next
CONSTRAINT InModel
INVARIANT Inv
VIEW View
