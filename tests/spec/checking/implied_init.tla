---- MODULE implied_init ----
\* A property that is a state predicate holds of each initial state; TLC
\* checks it with the invariants, as the initial states are computed.
EXTENDS Naturals
VARIABLE x
Init == x \in {0, 1}
Next == x' = 1 - x
P == x < 5
Q == x = 1
====
INIT Init
NEXT Next
PROPERTY P Q
