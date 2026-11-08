---- MODULE split_bound_error ----
\* An action \E q \in S : A whose S fails as TLC splits the next-state
\* action is one action, failing as the search takes it.
EXTENDS Naturals
VARIABLE x
S(p) == IF p = 2 THEN CHOOSE n \in {} : TRUE ELSE {p}
Go(p) == \E q \in S(p) : x' = q
Init == x = 0
Next == \E p \in 1..3 : Go(p)
====
INIT Init
NEXT Next
