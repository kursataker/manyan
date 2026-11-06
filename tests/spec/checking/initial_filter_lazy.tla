---- MODULE initial_filter_lazy ----
\* x \in {r \in S : P} takes the elements of S one at a time, as TLC
\* does: the first initial state violates the invariant before P fails on
\* the last element.
EXTENDS Naturals
VARIABLE x
Init == x \in {r \in [a : 1..3] : 1 \div (3 - r.a) >= 0}
Next == UNCHANGED x
Inv == x.a > 5
====
INIT Init
NEXT Next
INVARIANT Inv
